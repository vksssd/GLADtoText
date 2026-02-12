#pragma once
#include "../core/vector.h"
#include "../core/matrix.h"

class Embeddings {
public:
    Matrix E;

    explicit Embeddings(int vocab_size, int dim)
        : E(vocab_size, dim) {}

    Vector lookup(int id) const {
        return E.row(id);
    }

    void update(int id, const Vector& grad, float lr) {
        E.addRow(grad, id, lr);
    }
};
