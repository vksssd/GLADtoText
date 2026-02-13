#include "linear_classifier.h"
#include "utils/rng.h"
#include <cmath>
#include <cstring>

LinearClassifier::LinearClassifier(
    int input_dim,
    int num_classes,
    uint64_t seed)
    : input_dim_(input_dim),
      num_classes_(num_classes),
      weights_(num_classes * input_dim),
      bias_(num_classes)
{
    RNG rng(seed);

    float bound = std::sqrt(1.0f / input_dim_);

    for (auto& w : weights_)
        w = rng.uniform(-bound, bound);

    for (auto& b : bias_)
        b = 0.0f;
}

void LinearClassifier::forward(
    const float* input,
    float* logits) const
{
    for (int c = 0; c < num_classes_; ++c) {

        const float* row =
            &weights_[c * input_dim_];

        float sum = bias_[c];

        for (int j = 0; j < input_dim_; ++j)
            sum += row[j] * input[j];

        logits[c] = sum;
    }
}

void LinearClassifier::backward_sgd(
    const float* input,
    const float* dlogits,
    float* dinput,
    float learning_rate)
{
    // Optional gradient for sentence vector
    if (dinput) {
        std::memset(dinput, 0,
                    input_dim_ * sizeof(float));
    }

    for (int c = 0; c < num_classes_; ++c) {

        float grad_c = dlogits[c];

        float* row =
            &weights_[c * input_dim_];

        for (int j = 0; j < input_dim_; ++j) {

            if (dinput)
                dinput[j] += row[j] * grad_c;

            // SGD update
            row[j] -= learning_rate *
                      grad_c * input[j];
        }

        bias_[c] -= learning_rate * grad_c;
    }
}