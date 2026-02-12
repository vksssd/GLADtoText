#include "../heads/classify.h"
#include <iostream>

void trainSupervised(
    Backbone& backbone,
    Classifier& clf,
    const std::vector<std::vector<int>>& X,
    const std::vector<int>& y,
    int epochs
) {
    for (int ep = 0; ep < epochs; ep++) {
        float total_loss = 0.0f;
        for (size_t i = 0; i < X.size(); i++) {
            total_loss += clf.trainExample(X[i], y[i]);
        }
        std::cout << "Epoch " << ep
                  << " loss: " << total_loss / X.size()
                  << "\n";
    }
}


void trainSupervisedWithMemory(
    Backbone& backbone,
    Classifier& clf,
    ContextMemory& memory,
    const std::vector<std::vector<int>>& X,
    const std::vector<int>& y,
    int epochs
) {
    clf.attachMemory(&memory);

    for (int ep = 0; ep < epochs; ep++) {
        float total_loss = 0.0f;
        memory.reset();  // reset per epoch or per session

        for (size_t i = 0; i < X.size(); i++) {
            total_loss += clf.trainExample(X[i], y[i]);
        }

        std::cout << "Epoch " << ep
                  << " loss: " << total_loss / X.size()
                  << "\n";
    }
}
