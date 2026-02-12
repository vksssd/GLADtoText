#include <iostream>
#include "../model/backbone.h"
#include "../heads/semantic.h"

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

    std::cout << "Before training:\n";
    std::cout << "  emb[5][0] = " << emb.lookup(5).v[0] << "\n";
    std::cout << "  emb[2][0] = " << emb.lookup(2).v[0] << "\n";
    std::cout << "  att.w[0] = " << att.w.v[0] << "\n";
    std::cout << "  output[5][0] = " << trainer.output.row(5).v[0] << "\n";

    // Do one update
    trainer.cbowUpdate(ctx, target, sampler);

    std::cout << "\nAfter 1 update:\n";
    std::cout << "  emb[5][0] = " << emb.lookup(5).v[0] << "\n";
    std::cout << "  emb[2][0] = " << emb.lookup(2).v[0] << "\n";
    std::cout << "  att.w[0] = " << att.w.v[0] << "\n";
    std::cout << "  output[5][0] = " << trainer.output.row(5).v[0] << "\n";

    return 0;
}
