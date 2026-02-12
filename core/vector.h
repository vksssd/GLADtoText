#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

class Vector {
public:
    std::vector<float> v;

    explicit Vector(int dim = 0) : v(dim, 0.0f) {}

    // Zero out all elements
    void zero() {
        std::fill(v.begin(), v.end(), 0.0f);
    }

    // Add another vector with optional scaling
    void add(const Vector& other, float scale = 1.0f) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] += scale * other.v[i];
        }
    }

    // Scale all elements
    void scale(float s) {
        for (float& x : v) {
            x *= s;
        }
    }

    // Dot product
    float dot(const Vector& other) const {
        float sum = 0.0f;
        for (size_t i = 0; i < v.size(); i++) {
            sum += v[i] * other.v[i];
        }
        return sum;
    }

    // L2 norm (Euclidean length)
    float norm() const {
        return std::sqrt(dot(*this));
    }

    // Normalize to unit length
    void normalize() {
        float n = norm();
        if (n > 1e-8f) {
            scale(1.0f / n);
        }
    }

    // Element-wise multiplication
    void multiply(const Vector& other) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] *= other.v[i];
        }
    }

    // Cosine similarity
    float cosineSimilarity(const Vector& other) const {
        float dot_prod = dot(other);
        float norm_prod = norm() * other.norm();
        return (norm_prod > 1e-8f) ? (dot_prod / norm_prod) : 0.0f;
    }

    // L1 norm (Manhattan distance)
    float l1Norm() const {
        float sum = 0.0f;
        for (float x : v) {
            sum += std::abs(x);
        }
        return sum;
    }

    // Max element
    float max() const {
        return v.empty() ? 0.0f : *std::max_element(v.begin(), v.end());
    }

    // Min element
    float min() const {
        return v.empty() ? 0.0f : *std::min_element(v.begin(), v.end());
    }

    // Mean
    float mean() const {
        return v.empty() ? 0.0f : (std::accumulate(v.begin(), v.end(), 0.0f) / v.size());
    }

    // Clip values to range [min_val, max_val]
    void clip(float min_val, float max_val) {
        for (float& x : v) {
            x = std::max(min_val, std::min(max_val, x));
        }
    }

    // Apply ReLU activation
    void relu() {
        for (float& x : v) {
            x = std::max(0.0f, x);
        }
    }

    // Apply tanh activation
    void tanh_activation() {
        for (float& x : v) {
            x = std::tanh(x);
        }
    }

    // Apply sigmoid activation
    void sigmoid() {
        for (float& x : v) {
            x = 1.0f / (1.0f + std::exp(-x));
        }
    }

    // Size
    size_t size() const {
        return v.size();
    }

    // Resize
    void resize(size_t new_size) {
        v.resize(new_size, 0.0f);
    }
};

// Include matrix header after Vector is defined
#include "matrix.h"

// Matrix method implementations
inline Vector Matrix::row(int i) const {
    Vector v(cols);
    for (int j = 0; j < cols; j++) {
        v.v[j] = w[i * cols + j];
    }
    return v;
}

inline void Matrix::addRow(const Vector& grad, int i, float lr) {
    for (int j = 0; j < cols; j++) {
        w[i * cols + j] += lr * grad.v[j];
    }
}