#pragma once

#include <cmath>
#include <algorithm>

inline void softmax(float* logits,
                    int size)
{
    float max_val = logits[0];

    for (int i = 1; i < size; ++i)
        max_val = std::max(max_val, logits[i]);

    float sum = 0.0f;

    for (int i = 0; i < size; ++i) {
        logits[i] =
            std::exp(logits[i] - max_val);
        sum += logits[i];
    }

    for (int i = 0; i < size; ++i)
        logits[i] /= sum;
}

inline float cross_entropy(
    const float* probs,
    int true_class)
{
    const float eps = 1e-9f;
    float p = std::max(probs[true_class], eps);
    return -std::log(p);
}