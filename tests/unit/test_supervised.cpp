#include <iostream>
#include "../../heads/classify.h"

int main() {
    int vocab = 50;
    int dim = 8;

    Embeddings emb(vocab, dim);
    VectorAttention att(dim);
    Backbone bb(&emb, &att, dim);

    Classifier clf(3, dim, &bb);

    std::vector<int> sample = {1, 2, 3};

    float loss1 = clf.trainExample(sample, 1);

    for (int i = 0; i < 200; i++)
        clf.trainExample(sample, 1);

    float loss2 = clf.trainExample(sample, 1);

    std::cout << "Loss: " << loss1 << " → " << loss2 << "\n";

    if (loss2 < loss1)
        std::cout << "PASS\n";
    else
        std::cout << "FAIL\n";
}
