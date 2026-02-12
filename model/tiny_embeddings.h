#pragma once
#include "../core/vector.h"
#include "../core/sparse_matrix.h"
#include <random>

// Tiny embeddings with sparse storage
class TinyEmbeddings {
public:
    SparseMatrix E;
    int vocab_size;
    int dim;
    
    explicit TinyEmbeddings(int vocab, int d) 
        : E(vocab, d), vocab_size(vocab), dim(d) {
        // Initialize only frequently accessed embeddings
        // Others stay at zero (sparse)
    }
    
    Vector lookup(int id) const {
        return E.row(id);
    }
    
    void update(int id, const Vector& grad, float lr) {
        E.addRow(grad, id, lr);
    }
    
    // Initialize embedding on first access
    void initializeIfNeeded(int id) {
        bool has_values = false;
        for (int d = 0; d < dim; d++) {
            if (std::abs(E.get(id, d)) > 1e-6f) {
                has_values = true;
                break;
            }
        }
        
        if (!has_values) {
            // Lazy initialization
            std::mt19937 gen(id + 42);
            std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
            for (int d = 0; d < dim; d++) {
                E.set(id, d, dist(gen));
            }
        }
    }
    
    // Prune to save memory
    void prune(float threshold = 1e-4f) {
        E.prune(threshold);
    }
    
    size_t memoryUsage() const {
        return E.memoryUsage();
    }
};
