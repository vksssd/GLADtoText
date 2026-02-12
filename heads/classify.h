#pragma once
#include "../core/matrix.h"
#include "../model/backbone.h"
#include <cmath>
#include "../model/memory.h"



class Classifier {
public:
    Matrix prototypes;   // intent / class prototypes
    Backbone* backbone;
    int dim;
    int num_classes;
    float lr;
    float margin;
    // inside Classifier
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
          margin(m) {}

    static float softmaxDenom(const std::vector<float>& s) {
        float maxv = *std::max_element(s.begin(), s.end());
        float sum = 0.0f;
        for (float x : s) sum += std::exp(x - maxv);
        return sum;
    }

    float trainExample(const std::vector<int>& subword_ids, int label) {
        Vector h = backbone->forward(subword_ids);

        // compute scores
        std::vector<float> scores(num_classes);
        for (int k = 0; k < num_classes; k++) {
            scores[k] = prototypes.row(k).dot(h);
        }
        scores[label] -= margin;

        float denom = softmaxDenom(scores);
        float loss = -scores[label] + std::log(denom);

        // gradient wrt scores
        for (int k = 0; k < num_classes; k++) {
            float pk = std::exp(scores[k]) / denom;
            float grad = (k == label ? pk - 1.0f : pk);

            // update prototype
            Vector grad_p = h;
            grad_p.scale(-lr * grad);
            prototypes.addRow(grad_p, k, 1.0f);

            // backprop to h
            Vector grad_h = prototypes.row(k);
            grad_h.scale(-lr * grad);
            backpropToBackbone(subword_ids, grad_h);
        }

        return loss;
    }
    
    // Train with pre-computed vector (for sentence encoding)
    float trainExampleWithVector(const Vector& h, int label) {
        // compute scores
        std::vector<float> scores(num_classes);
        for (int k = 0; k < num_classes; k++) {
            scores[k] = prototypes.row(k).dot(h);
        }
        scores[label] -= margin;

        float denom = softmaxDenom(scores);
        float loss = -scores[label] + std::log(denom);

        // gradient wrt scores
        for (int k = 0; k < num_classes; k++) {
            float pk = std::exp(scores[k]) / denom;
            float grad = (k == label ? pk - 1.0f : pk);

            // update prototype
            Vector grad_p = h;
            grad_p.scale(-lr * grad);
            prototypes.addRow(grad_p, k, 1.0f);
        }

        return loss;
    }

private:
    void backpropToBackbone(const std::vector<int>& ids,
                            const Vector& grad_h) {
        // same logic as unsupervised backprop
        // distribute grad_h through attention and embeddings
        for (int id : ids) {
            Vector e = backbone->embeddings->lookup(id);
            for (int d = 0; d < dim; d++) {
                float a =
                    1.0f / (1.0f + std::exp(-backbone->attention->w.v[d] * e.v[d]));
                float da = a * (1 - a);

                // update attention
                backbone->attention->w.v[d] +=
                    lr * grad_h.v[d] * e.v[d] * da;

                // update embedding
                backbone->embeddings->E.w[id * dim + d] +=
                    lr * grad_h.v[d] * (a + e.v[d] * da * backbone->attention->w.v[d]);
            }
        }
    }
};
