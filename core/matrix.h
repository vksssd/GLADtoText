#pragma once
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

// Forward declaration - Vector is defined in vector.h
class Vector; 

class Matrix {
public:
    int rows, cols;
    std::vector<float> w;

    Matrix(int r, int c) : rows(r), cols(c), w(r * c) {
        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
        for (float& x : w) x = dist(gen);
    }

    // Get a row as a vector
    Vector row(int i) const;
    
    // Add gradient to a row
    void addRow(const Vector& grad, int i, float lr);
    
    // Get element at (i, j)
    float get(int i, int j) const {
        return w[i * cols + j];
    }
    
    // Set element at (i, j)
    void set(int i, int j, float val) {
        w[i * cols + j] = val;
    }
    
    // Add to element at (i, j)
    void add(int i, int j, float val) {
        w[i * cols + j] += val;
    }
    
    // Zero out all elements
    void zero() {
        std::fill(w.begin(), w.end(), 0.0f);
    }
    
    // Xavier/Glorot initialization
    void xavierInit() {
        std::mt19937 gen(42);
        float limit = std::sqrt(6.0f / (rows + cols));
        std::uniform_real_distribution<float> dist(-limit, limit);
        for (float& x : w) x = dist(gen);
    }
    
    // He initialization (for ReLU)
    void heInit() {
        std::mt19937 gen(42);
        float std_dev = std::sqrt(2.0f / rows);
        std::normal_distribution<float> dist(0.0f, std_dev);
        for (float& x : w) x = dist(gen);
    }
    
    // Clip all values to range
    void clip(float min_val, float max_val) {
        for (float& x : w) {
            x = std::max(min_val, std::min(max_val, x));
        }
    }
    
    // L2 regularization (weight decay)
    void l2Regularize(float lambda, float lr) {
        for (float& x : w) {
            x *= (1.0f - lambda * lr);
        }
    }
    
    // Get memory usage in bytes
    size_t memoryUsage() const {
        return w.size() * sizeof(float);
    }
    
    // Prune small weights
    void prune(float threshold = 1e-4f) {
        for (float& x : w) {
            if (std::abs(x) < threshold) {
                x = 0.0f;
            }
        }
    }
};