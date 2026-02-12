#pragma once
#include "../core/vector.h"
#include <cmath>
#include <algorithm>

class VectorAttention {
public:
    Vector w;  // attention vector

    explicit VectorAttention(int dim) : w(dim) {
        // Initialize with small random values for better training
        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(-0.01f, 0.01f);
        for (float& val : w.v) {
            val = dist(gen);
        }
    }

    // Numerically stable sigmoid
    static float sigmoid(float x) {
        if (x >= 0.0f) {
            float z = std::exp(-x);
            return 1.0f / (1.0f + z);
        } else {
            float z = std::exp(x);
            return z / (1.0f + z);
        }
    }

    // Apply attention with numerical stability
    Vector apply(const Vector& e) const {
        Vector out(e.v.size());
        for (size_t i = 0; i < e.v.size(); i++) {
            float score = w.v[i] * e.v[i];
            // Clip to prevent overflow
            score = std::max(-10.0f, std::min(10.0f, score));
            float a = sigmoid(score);
            out.v[i] = a * e.v[i];
        }
        return out;
    }
    
    // Update attention weights with gradient clipping
    void update(const Vector& grad, float lr, float clip_val = 5.0f) {
        for (size_t i = 0; i < w.v.size(); i++) {
            float g = grad.v[i];
            // Gradient clipping
            g = std::max(-clip_val, std::min(clip_val, g));
            w.v[i] += lr * g;
        }
    }
};
