#include <iostream>
#include "../../model/backbone.h"

int main() {
    int vocab = 50;
    int dim = 8;

    Embeddings emb(vocab, dim);
    VectorAttention att(dim);
    Backbone bb(&emb, &att, dim);

    std::vector<int> ids = {2, 5, 10, 20};

    Vector h1 = bb.forward(ids);

    // Modify attention → output must change
    att.w.v[0] = 5.0f;

    Vector h2 = bb.forward(ids);

    std::cout << "Backbone test:\n";
    std::cout << "h1[0] = " << h1.v[0] << "\n";
    std::cout << "h2[0] = " << h2.v[0] << "\n";

    if (h1.v[0] != h2.v[0])
        std::cout << "PASS\n";
    else
        std::cout << "FAIL\n";
}
