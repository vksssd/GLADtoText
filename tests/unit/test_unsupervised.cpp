#include <iostream>
#include "../../model/backbone.h"
#include "../../heads/semantic.h"

int main() {
    int vocab = 100;
    int dim = 16;

    Embeddings emb(vocab, dim);
    VectorAttention att(dim);
    Backbone bb(&emb, &att, dim);

    SemanticTrainer trainer(vocab, dim, &bb, 0.05f, 5);
    NegativeSampler sampler(vocab);

    std::vector<int> ctx = {2, 3};
    int target = 5;

    float before = emb.lookup(2).v[0];  // Check context embedding, not target

    for (int i = 0; i < 100; i++)
        trainer.cbowUpdate(ctx, target, sampler);

    float after = emb.lookup(2).v[0];

    std::cout << "Context embedding change: " << before << " → " << after << "\n";

    if (before != after)
        std::cout << "PASS\n";
    else
        std::cout << "FAIL\n";
}
