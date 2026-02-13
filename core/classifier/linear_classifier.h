#pragma once

#include <vector>
#include <cstdint>

class LinearClassifier {
    public:
        LinearClassifier(int input_dim, int num_classes, uint64_t seed);

        
        void forward(const float* input, float* logits) const;

        void backward_sgd(const float* input,  const float* dlogits, float* dinput, float learning_rate);

        int input_dim() const noexcept { return input_dim_; }
        int num_classes() const noexcept { return num_classes_; }
    private:
        int input_dim_;
        int num_classes_;

        std::vector<float> weights_;
        std::vector<float> bias_;
};