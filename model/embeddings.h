#pragma once
#include "../core/vector.h"
#include "../core/matrix.h"
#include <stdexcept>

class Embeddings {
public:
    Matrix E;

    explicit Embeddings(int vocab_size, int dim)
        : E(vocab_size, dim) {
        if (vocab_size <= 0 || dim <= 0) {
            throw std::invalid_argument("Vocab size and dimension must be positive");
        }
    }

    // Lookup with bounds checking
    Vector lookup(int id) const {
        if (id < 0 || id >= E.rows) {
            // Return zero vector for out-of-bounds access
            Vector zero(E.cols);
            zero.zero();
            return zero;
        }
        return E.row(id);
    }

    // Update with bounds checking
    void update(int id, const Vector& grad, float lr) {
        if (id >= 0 && id < E.rows) {
            E.addRow(grad, id, lr);
        }
    }
    
    // Get vocabulary size
    int vocabSize() const {
        return E.rows;
    }
    
    // Get embedding dimension
    int embeddingDim() const {
        return E.cols;
    }
};
