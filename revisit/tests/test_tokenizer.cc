#include <gtest/gtest.h>
#include "tokenizer/english_tokenizer.h"

TEST(TokenizerTest, BasicTokenization) {
    EnglishTokenizer tokenizer;
    std::vector<std::string> tokens;
    
    tokenizer.tokenize("Hello world", tokens);
    
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST(TokenizerTest, Punctuation) {
    EnglishTokenizer tokenizer;
    std::vector<std::string> tokens;
    
    tokenizer.tokenize("Hello, world!", tokens);
    
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST(TokenizerTest, EmptyString) {
    EnglishTokenizer tokenizer;
    std::vector<std::string> tokens;
    
    tokenizer.tokenize("", tokens);
    
    EXPECT_EQ(tokens.size(), 0);
}

TEST(TokenizerTest, MultipleSpaces) {
    EnglishTokenizer tokenizer;
    std::vector<std::string> tokens;
    
    tokenizer.tokenize("hello    world", tokens);
    
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}
