#pragma once
#include "../core/vector.h"
#include <cmath>
#include <algorithm>

// Attention modes for performance tuning
enum class AttentionMode {
    FULL,      // Full sigmoid-based attention (default, most accurate)
    SIMPLE,    // Linear attention without sigmoid (faster)
    NONE       // No attention, direct averaging (fastest)
};

class VectorAttention {
public:
    Vector w;  // attention vector
    AttentionMode mode;  // attention mode

    explicit VectorAttention(int dim, AttentionMode m = AttentionMode::FULL) 
        : w(dim), mode(m) {
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
        
        switch (mode) {
            case AttentionMode::FULL: {
                // Full sigmoid-based attention (most accurate)
                for (size_t i = 0; i < e.v.size(); i++) {
                    float score = w.v[i] * e.v[i];
                    // Clip to prevent overflow
                    score = std::max(-10.0f, std::min(10.0f, score));
                    float a = sigmoid(score);
                    out.v[i] = a * e.v[i];
                }
                break;
            }
            
            case AttentionMode::SIMPLE: {
                // Simple linear attention (faster, no sigmoid)
                for (size_t i = 0; i < e.v.size(); i++) {
                    float score = w.v[i] * e.v[i];
                    // Clip to prevent overflow
                    score = std::max(-1.0f, std::min(1.0f, score));
                    out.v[i] = (1.0f + score) * e.v[i];  // Scale by (1 + score)
                }
                break;
            }
            
            case AttentionMode::NONE: {
                // No attention, direct pass-through (fastest)
                out = e;
                break;
            }
        }
        
        return out;
    }
    
    // Update attention weights with gradient clipping
    void update(const Vector& grad, float lr, float clip_val = 5.0f) {
        // Skip updates if attention is disabled
        if (mode == AttentionMode::NONE) {
            return;
        }
        
        for (size_t i = 0; i < w.v.size(); i++) {
            float g = grad.v[i];
            // Gradient clipping
            g = std::max(-clip_val, std::min(clip_val, g));
            w.v[i] += lr * g;
        }
    }
};
