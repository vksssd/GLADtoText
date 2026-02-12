#pragma once
#include "../core/vector.h"
#include <cmath>

class ContextMemory {
public:
    Vector mem0;   // L0
    Vector mem1;   // L1

    Vector gate0;  // gate vectors
    Vector gate1;

    float decay0;  // alpha0
    float decay1;  // alpha1

    float beta0;
    float beta1;

    explicit ContextMemory(int dim)
        : mem0(dim), mem1(dim),
          gate0(dim), gate1(dim),
          decay0(0.3f), decay1(0.9f),
          beta0(0.6f), beta1(0.4f) {

        mem0.zero();
        mem1.zero();
    }

    static float sigmoid(float x) {
        return 1.0f / (1.0f + std::exp(-x));
    }

    // compute gated memory injection
    Vector apply(const Vector& h) {
        float g0 = sigmoid(gate0.dot(h));
        float g1 = sigmoid(gate1.dot(h));

        Vector out = h;
        for (size_t i = 0; i < h.v.size(); i++) {
            out.v[i] += beta0 * g0 * mem0.v[i];
            out.v[i] += beta1 * g1 * mem1.v[i];
        }
        return out;
    }

    // update memory AFTER classification
    void update(const Vector& h) {
        float g0 = sigmoid(gate0.dot(h));

        for (size_t i = 0; i < h.v.size(); i++) {
            mem0.v[i] = decay0 * mem0.v[i] +
                        (1.0f - decay0) * h.v[i];

            mem1.v[i] = decay1 * mem1.v[i] +
                        (1.0f - decay1) * g0 * mem0.v[i];
        }
    }

    void reset() {
        mem0.zero();
        mem1.zero();
    }
};
