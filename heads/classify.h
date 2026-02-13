#pragma once
#include "../core/matrix.h"
#include "../model/backbone.h"
#include "../model/sentence_encoder.h"
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
    
    // Train with sentence encoding (includes backprop to sentence encoder)
    float trainExampleWithSentence(const std::vector<Vector>& word_embeddings,
                                   const std::vector<std::vector<int>>& word_ids,
                                   SentenceEncoder* sentEnc,
                                   int label) {
        if (label < 0 || label >= num_classes) {
            return 0.0f;
        }
        if (word_embeddings.empty()) {
            return 0.0f;
        }

        // Forward pass through sentence encoder
        Vector h = sentEnc->encode(word_embeddings);

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

        // Compute gradient wrt sentence vector
        Vector grad_h(dim);
        grad_h.zero();
        
        for (int k = 0; k < num_classes; k++) {
            float pk = std::exp(scores[k] - maxScore) / denom;
            float grad = (k == label ? pk - 1.0f : pk);
            grad = std::max(-5.0f, std::min(5.0f, grad));

            // Update prototype
            Vector grad_p = h;
            grad_p.scale(-lr * grad);
            prototypes.addRow(grad_p, k, 1.0f);

            // Accumulate gradient for sentence vector
            Vector proto_grad = prototypes.row(k);
            proto_grad.scale(-lr * grad);
            grad_h.add(proto_grad);
        }

        // Backprop through sentence encoder
        sentEnc->update(word_embeddings, grad_h, lr);
        
        // Backprop to word embeddings (through backbone)
        for (size_t i = 0; i < word_ids.size(); i++) {
            if (!word_ids[i].empty()) {
                backpropToBackbone(word_ids[i], grad_h);
            }
        }

        return loss;
    }
    
    // Batch training with subword IDs (processes batch sequentially)
    float trainBatch(const std::vector<std::vector<int>>& batch_ids, 
                     const std::vector<int>& batch_labels) {
        int batchSize = batch_ids.size();
        if (batchSize == 0 || batchSize != static_cast<int>(batch_labels.size())) {
            return 0.0f;
        }

        float totalLoss = 0.0f;
        int validExamples = 0;

        for (int i = 0; i < batchSize; i++) {
            if (!batch_ids[i].empty() && batch_labels[i] >= 0 && batch_labels[i] < num_classes) {
                float loss = trainExample(batch_ids[i], batch_labels[i]);
                totalLoss += loss;
                validExamples++;
            }
        }

        return validExamples > 0 ? (totalLoss / validExamples) : 0.0f;
    }

    // Batch training with pre-computed vectors (processes batch sequentially)
    float trainBatchWithVectors(const std::vector<Vector>& batch_h, 
                                const std::vector<int>& batch_labels) {
        int batchSize = batch_h.size();
        if (batchSize == 0 || batchSize != static_cast<int>(batch_labels.size())) {
            return 0.0f;
        }

        float totalLoss = 0.0f;
        int validExamples = 0;

        for (int i = 0; i < batchSize; i++) {
            if (batch_labels[i] >= 0 && batch_labels[i] < num_classes) {
                float loss = trainExampleWithVector(batch_h[i], batch_labels[i]);
                totalLoss += loss;
                validExamples++;
            }
        }

        return validExamples > 0 ? (totalLoss / validExamples) : 0.0f;
    }
    
    // Batch training with sentence encoding (includes backprop)
    float trainBatchWithSentences(const std::vector<std::vector<Vector>>& batch_word_embeddings,
                                  const std::vector<std::vector<std::vector<int>>>& batch_word_ids,
                                  SentenceEncoder* sentEnc,
                                  const std::vector<int>& batch_labels) {
        int batchSize = batch_word_embeddings.size();
        if (batchSize == 0 || batchSize != static_cast<int>(batch_labels.size())) {
            return 0.0f;
        }

        float totalLoss = 0.0f;
        int validExamples = 0;

        for (int i = 0; i < batchSize; i++) {
            if (batch_labels[i] >= 0 && batch_labels[i] < num_classes && 
                !batch_word_embeddings[i].empty()) {
                float loss = trainExampleWithSentence(
                    batch_word_embeddings[i],
                    batch_word_ids[i],
                    sentEnc,
                    batch_labels[i]
                );
                totalLoss += loss;
                validExamples++;
            }
        }

        return validExamples > 0 ? (totalLoss / validExamples) : 0.0f;
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
        // NOTE: grad_h is already scaled by learning rate in trainExample
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
                // grad_h is already scaled by lr, so don't multiply by lr again
                float grad_att = grad_h.v[d] * e.v[d] * da;
                grad_att = std::max(-1.0f, std::min(1.0f, grad_att));
                backbone->attention->w.v[d] += grad_att;

                // Update embedding with gradient clipping
                float grad_emb = grad_h.v[d] * (a + e.v[d] * da * backbone->attention->w.v[d]);
                grad_emb = std::max(-1.0f, std::min(1.0f, grad_emb));
                backbone->embeddings->E.w[id * dim + d] += grad_emb;
            }
        }
    }
};
