#include <gtest/gtest.h>
#include "loss/softmax.h"

TEST(SoftmaxTest, ProbabilitiesSumToOne) {

    float logits[3] = {1.0f, 2.0f, 3.0f};

    softmax(logits, 3);

    float sum = logits[0] + logits[1] + logits[2];

    EXPECT_NEAR(sum, 1.0f, 1e-5);
}

TEST(SoftmaxTest, NoNaNWithLargeValues) {

    float logits[3] = {1000.0f, 2000.0f, 3000.0f};

    softmax(logits, 3);

    for (int i = 0; i < 3; ++i)
        EXPECT_FALSE(std::isnan(logits[i]));
}

TEST(CrossEntropyTest, LossIsPositive) {

    float probs[2] = {0.2f, 0.8f};

    float loss = cross_entropy(probs, 1);

    EXPECT_GT(loss, 0.0f);
}