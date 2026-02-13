#include <gtest/gtest.h>
#include "config/model_config.h"

TEST(ModelConfigTest, DefaultValues){
    ModelConfig config;
    EXPECT_EQ(config.bucket_count, 200000);
    EXPECT_EQ(config.embedding_dim, 256);
}

TEST(ModelConfigTest, EqualityOperator){
    ModelConfig a;
    ModelConfig b;
    EXPECT_TRUE(a==b);
}