#pragma once
#include "../model/embeddings.h"
#include "../model/attention.h"
#include <vector>

class Backbone {
public:
    Embeddings* embeddings;
    VectorAttention* attention;
    int dim;

    Backbone(Embeddings* E, VectorAttention* A, int d)
        : embeddings(E), attention(A), dim(d) {}

    Vector forward(const std::vector<int>& subword_ids) const {
        Vector h(dim);
        h.zero();

        for (int id : subword_ids) {
            Vector e = embeddings->lookup(id);
            Vector ae = attention->apply(e);
            h.add(ae);
        }

        return h;
    }
};
