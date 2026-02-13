#include <gtest/gtest.h>
#include "utils/rng.h"

TEST(RNGTest, DeterministicUniform){
    RNG rng1(42);
    RNG rng2(42);

    float v1 = rng1.uniform(0.0f, 1.0f);
    float v2 = rng2.uniform(0.0f, 1.0f);
    
    EXPECT_FLOAT_EQ(v1,v2);
}