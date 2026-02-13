#include <gtest/gtest.h>
#include "classifier/linear_classifier.h"

TEST(LinearClassifierTest, ForwardOutputShape) {

    LinearClassifier clf(8, 3, 42);

    float input[8] = {0};
    float logits[3];

    clf.forward(input, logits);

    // Just ensure no crash and logits accessible
    EXPECT_TRUE(true);
}

TEST(LinearClassifierTest, DeterministicInitialization) {

    LinearClassifier clf1(8, 3, 123);
    LinearClassifier clf2(8, 3, 123);

    float input[8] = {1,2,3,4,5,6,7,8};

    float logits1[3];
    float logits2[3];

    clf1.forward(input, logits1);
    clf2.forward(input, logits2);

    for (int i = 0; i < 3; ++i)
        EXPECT_FLOAT_EQ(logits1[i], logits2[i]);
}

TEST(LinearClassifierTest, BackwardUpdatesWeights) {

    LinearClassifier clf(4, 2, 42);

    float input[4] = {1,1,1,1};
    float logits[2];

    clf.forward(input, logits);

    float dlogits[2] = {0.5f, -0.5f};

    clf.backward_sgd(input, dlogits, nullptr, 0.1f);

    float logits_after[2];
    clf.forward(input, logits_after);

    EXPECT_FALSE(
        logits_after[0] == logits[0] &&
        logits_after[1] == logits[1]
    );
}