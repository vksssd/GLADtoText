#pragma once
#include "../core/matrix.h"
#include "../model/backbone.h"
#include <random>
#include <cmath>

/* ---------- Utilities ---------- */

inline float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

/* ---------- Negative Sampler ---------- */

class NegativeSampler {
public:
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist;

    explicit NegativeSampler(int vocab)
        : gen(123), dist(0, vocab - 1) {}

    int sample() { return dist(gen); }
};

/* ---------- Semantic Trainer ---------- */

class SemanticTrainer {
public:
    Matrix output;          // output embeddings
    Backbone* backbone;
    int dim;
    float lr;
    int negK;

    SemanticTrainer(int vocab,
                    int d,
                    Backbone* bb,
                    float lr_,
                    int neg)
        : output(vocab, d),
          backbone(bb),
          dim(d),
          lr(lr_),
          negK(neg) {}

    /* ---------- CBOW ---------- */
    void cbowUpdate(const std::vector<int>& context_subword_ids,
                    int target_word_id,
                    NegativeSampler& sampler) {

        // forward
        Vector h = backbone->forward(context_subword_ids);

        // positive
        updateOne(context_subword_ids, h, target_word_id, +1.0f);

        // negatives
        for (int i = 0; i < negK; i++) {
            int neg = sampler.sample();
            if (neg != target_word_id) {
                updateOne(context_subword_ids, h, neg, -1.0f);
            }
        }
    }

    /* ---------- Skip-gram ---------- */
    void skipgramUpdate(const std::vector<int>& center_subword_ids,
                        const std::vector<int>& context_word_ids,
                        NegativeSampler& sampler) {

        Vector h = backbone->forward(center_subword_ids);

        for (int ctx : context_word_ids) {
            updateOne(center_subword_ids, h, ctx, +1.0f);

            for (int i = 0; i < negK; i++) {
                int neg = sampler.sample();
                if (neg != ctx) {
                    updateOne(center_subword_ids, h, neg, -1.0f);
                }
            }
        }
    }

private:
    void updateOne(const std::vector<int>& subword_ids,
                   const Vector& h,
                   int word,
                   float label) {

        // score
        Vector u = output.row(word);
        float s = u.dot(h);
        float p = sigmoid(label * s);
        float g = lr * (label * (1.0f - p));

        // update output embedding
        Vector grad_u = h;
        grad_u.scale(g);
        output.addRow(grad_u, word, 1.0f);

        // backprop to backbone
        Vector grad_h = u;
        grad_h.scale(g);
        backpropToBackbone(subword_ids, grad_h);
    }

    void backpropToBackbone(const std::vector<int>& subword_ids,
                            const Vector& grad_h) {

        for (int id : subword_ids) {
            Vector e = backbone->embeddings->lookup(id);

            for (int d = 0; d < dim; d++) {
                float wd = backbone->attention->w.v[d];
                float ed = e.v[d];

                float a = sigmoid(wd * ed);
                float da = a * (1.0f - a);

                // update attention
                backbone->attention->w.v[d] +=
                    lr * grad_h.v[d] * ed * da;

                // update embedding
                backbone->embeddings->E.w[id * dim + d] +=
                    lr * grad_h.v[d] * (a + ed * da * wd);
            }
        }
    }
};
