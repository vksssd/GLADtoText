#include <gtest/gtest.h>
#include "ngram/ngram_generator.h"

TEST(NGramGeneratorTest, BasicGeneration) {
    NGramGenerator gen(3, 6);
    
    std::string wrapped;
    std::vector<std::string_view> ngrams;
    
    gen.generate("hello", wrapped, ngrams);
    
    EXPECT_EQ(wrapped, "<hello>");
    EXPECT_GT(ngrams.size(), 0);
}

TEST(NGramGeneratorTest, CorrectNGrams) {
    NGramGenerator gen(3, 3);
    
    std::string wrapped;
    std::vector<std::string_view> ngrams;
    
    gen.generate("cat", wrapped, ngrams);
    
    // <cat> should generate: <ca, cat, at>
    EXPECT_EQ(ngrams.size(), 3);
}

TEST(NGramGeneratorTest, MinMaxRange) {
    NGramGenerator gen(2, 4);
    
    EXPECT_EQ(gen.min_n(), 2);
    EXPECT_EQ(gen.max_n(), 4);
}
