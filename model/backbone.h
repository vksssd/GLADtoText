#pragma once
#include "../model/embeddings.h"
#include "../model/attention.h"
#include <vector>
#include <stdexcept>

class Backbone {
public:
    Embeddings* embeddings;
    VectorAttention* attention;
    int dim;

    Backbone(Embeddings* E, VectorAttention* A, int d)
        : embeddings(E), attention(A), dim(d) {
        if (!E || !A) {
            throw std::invalid_argument("Embeddings and Attention cannot be null");
        }
        if (d <= 0) {
            throw std::invalid_argument("Dimension must be positive");
        }
    }

    // Forward pass with averaging
    Vector forward(const std::vector<int>& subword_ids) const {
        Vector h(dim);
        h.zero();

        if (subword_ids.empty()) {
            return h;  // Return zero vector for empty input
        }

        for (int id : subword_ids) {
            if (id < 0) continue;  // Skip invalid IDs
            
            Vector e = embeddings->lookup(id);
            Vector ae = attention->apply(e);
            h.add(ae);
        }

        // Average instead of sum for better numerical stability
        h.scale(1.0f / subword_ids.size());

        return h;
    }
    
    // Forward pass without averaging (for compatibility)
    Vector forwardSum(const std::vector<int>& subword_ids) const {
        Vector h(dim);
        h.zero();

        for (int id : subword_ids) {
            if (id < 0) continue;
            
            Vector e = embeddings->lookup(id);
            Vector ae = attention->apply(e);
            h.add(ae);
        }

        return h;
    }
};
