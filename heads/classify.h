#pragma once
#include "../core/matrix.h"
#include "../model/backbone.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "../model/memory.h"

class Classifier {
public:
    Matrix prototypes;   // intent / class prototypes
    Backbone* backbone;
    int dim;
    int num_classes;
    float lr;
    float margin;
    ContextMemory* memory = nullptr;

    void attachMemory(ContextMemory* m) {
        memory = m;
    }

    Classifier(int classes, int d, Backbone* bb,
               float lr_ = 0.05f, float m = 0.2f)
        : prototypes(classes, d),
          backbone(bb),
          dim(d),
          num_classes(classes),
          lr(lr_),
          margin(m) {
        if (classes <= 0 || d <= 0) {
            throw std::invalid_argument("Classes and dimension must be positive");
        }
        if (!bb) {
            throw std::invalid_argument("Backbone cannot be null");
        }
    }

    // Numerically stable softmax denominator
    static float softmaxDenom(const std::vector<float>& s) {
        if (s.empty()) return 1.0f;
        
        float maxv = *std::max_element(s.begin(), s.end());
        float sum = 0.0f;
        for (float x : s) {
            sum += std::exp(x - maxv);
        }
        return sum;
    }

    // Train with subword IDs
    float trainExample(const std::vector<int>& subword_ids, int label) {
        if (label < 0 || label >= num_classes) {
            return 0.0f;  // Invalid label
        }
        if (subword_ids.empty()) {
            return 0.0f;  // Empty input
        }

        Vector h = backbone->forward(subword_ids);

        // Compute scores
        std::vector<float> scores(num_classes);
        for (int k = 0; k < num_classes; k++) {
            scores[k] = prototypes.row(k).dot(h);
        }
        scores[label] -= margin;

        // Numerically stable loss computation
        float maxScore = *std::max_element(scores.begin(), scores.end());
        float denom = 0.0f;
        for (float s : scores) {
            denom += std::exp(s - maxScore);
        }
        float loss = -scores[label] + maxScore + std::log(denom);

        // Gradient wrt scores
        for (int k = 0; k < num_classes; k++) {
            float pk = std::exp(scores[k] - maxScore) / denom;
            float grad = (k == label ? pk - 1.0f : pk);

            // Gradient clipping
            grad = std::max(-5.0f, std::min(5.0f, grad));

            // Update prototype
            Vector grad_p = h;
            grad_p.scale(-lr * grad);
            prototypes.addRow(grad_p, k, 1.0f);

            // Backprop to h
            Vector grad_h = prototypes.row(k);
            grad_h.scale(-lr * grad);
            backpropToBackbone(subword_ids, grad_h);
        }

        return loss;
    }
    
    // Train with pre-computed vector (for sentence encoding)
    float trainExampleWithVector(const Vector& h, int label) {
        if (label < 0 || label >= num_classes) {
            return 0.0f;
        }

        // Compute scores
        std::vector<float> scores(num_classes);
        for (int k = 0; k < num_classes; k++) {
            scores[k] = prototypes.row(k).dot(h);
        }
        scores[label] -= margin;

        // Numerically stable loss
        float maxScore = *std::max_element(scores.begin(), scores.end());
        float denom = 0.0f;
        for (float s : scores) {
            denom += std::exp(s - maxScore);
        }
        float loss = -scores[label] + maxScore + std::log(denom);

        // Gradient wrt scores
        for (int k = 0; k < num_classes; k++) {
            float pk = std::exp(scores[k] - maxScore) / denom;
            float grad = (k == label ? pk - 1.0f : pk);

            // Gradient clipping
            grad = std::max(-5.0f, std::min(5.0f, grad));

            // Update prototype
            Vector grad_p = h;
            grad_p.scale(-lr * grad);
            prototypes.addRow(grad_p, k, 1.0f);
        }

        return loss;
    }
    
    // Predict with scores
    std::vector<std::pair<int, float>> predictTopK(const Vector& h, int k = 1) const {
        std::vector<std::pair<int, float>> results;
        
        for (int i = 0; i < num_classes; i++) {
            float score = prototypes.row(i).dot(h);
            results.push_back({i, score});
        }
        
        // Sort by score descending
        std::sort(results.begin(), results.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (k < static_cast<int>(results.size())) {
            results.resize(k);
        }
        
        return results;
    }

private:
    void backpropToBackbone(const std::vector<int>& ids, const Vector& grad_h) {
        // Distribute gradient through attention and embeddings
        for (int id : ids) {
            if (id < 0) continue;  // Skip invalid IDs
            
            Vector e = backbone->embeddings->lookup(id);
            for (int d = 0; d < dim; d++) {
                float score = backbone->attention->w.v[d] * e.v[d];
                // Clip to prevent overflow
                score = std::max(-10.0f, std::min(10.0f, score));
                
                float a = 1.0f / (1.0f + std::exp(-score));
                float da = a * (1.0f - a);

                // Update attention with gradient clipping
                float grad_att = lr * grad_h.v[d] * e.v[d] * da;
                grad_att = std::max(-1.0f, std::min(1.0f, grad_att));
                backbone->attention->w.v[d] += grad_att;

                // Update embedding with gradient clipping
                float grad_emb = lr * grad_h.v[d] * (a + e.v[d] * da * backbone->attention->w.v[d]);
                grad_emb = std::max(-1.0f, std::min(1.0f, grad_emb));
                backbone->embeddings->E.w[id * dim + d] += grad_emb;
            }
        }
    }
};
