#pragma once
#include "../core/vector.h"
#include <cmath>

class VectorAttention {
public:
    Vector w;  // attention vector

    explicit VectorAttention(int dim) : w(dim) {}

    static float sigmoid(float x) {
        return 1.0f / (1.0f + std::exp(-x));
    }

    Vector apply(const Vector& e) const {
        Vector out(e.v.size());
        for (size_t i = 0; i < e.v.size(); i++) {
            float a = sigmoid(w.v[i] * e.v[i]);
            out.v[i] = a * e.v[i];
        }
        return out;
    }
};
