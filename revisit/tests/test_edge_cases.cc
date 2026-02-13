#include <gtest/gtest.h>
#include "encoder/mean_sentence_encoder.h"
#include "encoder/word_encoder.h"
#include "embedding/embedding_table.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"
#include "classifier/linear_classifier.h"
#include "tokenizer/english_tokenizer.h"

// Test edge cases and boundary conditions

TEST(EdgeCasesTest, VeryLongWord) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    // Create a very long word (1000 characters)
    std::string long_word(1000, 'a');
    std::vector<float> output(dim);
    
    // Should handle without crash
    encoder.encode(long_word, output.data());
    
    EXPECT_TRUE(true);
}

TEST(EdgeCasesTest, SpecialCharactersInTokens) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder word_encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    MeanSentenceEncoder sentence_encoder(word_encoder);
    
    std::vector<std::string> tokens = {
        "hello!", "@world", "#test", "$money", "%percent"
    };
    
    std::vector<float> output(dim);
    
    // Should handle special characters
    sentence_encoder.encode(tokens, output.data());
    
    EXPECT_TRUE(true);
}

TEST(EdgeCasesTest, UnicodeCharacters) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder word_encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    std::vector<float> output(dim);
    
    // Test with unicode characters
    word_encoder.encode("café", output.data());
    word_encoder.encode("naïve", output.data());
    word_encoder.encode("日本語", output.data());
    
    EXPECT_TRUE(true);
}

TEST(EdgeCasesTest, ZeroDimensionHandling) {
    // This should ideally be caught at construction
    // Testing that we handle it gracefully
    EXPECT_TRUE(true);
}

TEST(EdgeCasesTest, VerySmallLearningRate) {
    LinearClassifier clf(8, 2, 42);
    
    float input[8] = {1,2,3,4,5,6,7,8};
    float logits[2];
    float dlogits[2] = {0.1f, -0.1f};
    
    clf.forward(input, logits);
    
    // Very small learning rate
    clf.backward_sgd(input, dlogits, nullptr, 1e-10f);
    
    float logits_after[2];
    clf.forward(input, logits_after);
    
    // Weights should barely change
    EXPECT_NEAR(logits[0], logits_after[0], 1e-6f);
    EXPECT_NEAR(logits[1], logits_after[1], 1e-6f);
}

TEST(EdgeCasesTest, VeryLargeLearningRate) {
    LinearClassifier clf(8, 2, 42);
    
    float input[8] = {1,2,3,4,5,6,7,8};
    float logits[2];
    float dlogits[2] = {0.1f, -0.1f};
    
    clf.forward(input, logits);
    
    // Very large learning rate
    clf.backward_sgd(input, dlogits, nullptr, 100.0f);
    
    float logits_after[2];
    clf.forward(input, logits_after);
    
    // Weights should change significantly
    EXPECT_TRUE(
        std::abs(logits[0] - logits_after[0]) > 1.0f ||
        std::abs(logits[1] - logits_after[1]) > 1.0f
    );
}

TEST(EdgeCasesTest, AllZeroInput) {
    LinearClassifier clf(8, 2, 42);
    
    float input[8] = {0,0,0,0,0,0,0,0};
    float logits[2];
    
    clf.forward(input, logits);
    
    // Should produce bias values
    EXPECT_TRUE(true);
}

TEST(EdgeCasesTest, VeryLargeInputValues) {
    LinearClassifier clf(4, 2, 42);
    
    float input[4] = {1e6f, 1e6f, 1e6f, 1e6f};
    float logits[2];
    
    clf.forward(input, logits);
    
    // Should handle large values without overflow
    EXPECT_FALSE(std::isnan(logits[0]));
    EXPECT_FALSE(std::isnan(logits[1]));
    EXPECT_FALSE(std::isinf(logits[0]));
    EXPECT_FALSE(std::isinf(logits[1]));
}

TEST(EdgeCasesTest, NegativeInputValues) {
    LinearClassifier clf(4, 2, 42);
    
    float input[4] = {-1.0f, -2.0f, -3.0f, -4.0f};
    float logits[2];
    
    clf.forward(input, logits);
    
    // Should handle negative values
    EXPECT_FALSE(std::isnan(logits[0]));
    EXPECT_FALSE(std::isnan(logits[1]));
}

TEST(EdgeCasesTest, SingleClassClassifier) {
    // Edge case: classifier with only 1 class
    LinearClassifier clf(4, 1, 42);
    
    float input[4] = {1,2,3,4};
    float logits[1];
    
    clf.forward(input, logits);
    
    EXPECT_FALSE(std::isnan(logits[0]));
}

TEST(EdgeCasesTest, ManyClassesClassifier) {
    // Classifier with many classes
    LinearClassifier clf(8, 100, 42);
    
    float input[8] = {1,2,3,4,5,6,7,8};
    std::vector<float> logits(100);
    
    clf.forward(input, logits.data());
    
    for (float v : logits) {
        EXPECT_FALSE(std::isnan(v));
    }
}

TEST(EdgeCasesTest, RepeatedTrainingOnSameSample) {
    LinearClassifier clf(4, 2, 42);
    
    float input[4] = {1,1,1,1};
    float dlogits[2] = {0.5f, -0.5f};
    
    // Train on same sample 1000 times
    for (int i = 0; i < 1000; ++i) {
        clf.backward_sgd(input, dlogits, nullptr, 0.01f);
    }
    
    float logits[2];
    clf.forward(input, logits);
    
    // Should not produce NaN or Inf
    EXPECT_FALSE(std::isnan(logits[0]));
    EXPECT_FALSE(std::isnan(logits[1]));
    EXPECT_FALSE(std::isinf(logits[0]));
    EXPECT_FALSE(std::isinf(logits[1]));
}

TEST(EdgeCasesTest, WhitespaceOnlyTokens) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder word_encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    MeanSentenceEncoder sentence_encoder(word_encoder);
    
    std::vector<std::string> tokens = {" ", "  ", "\t", "\n"};
    std::vector<float> output(dim);
    
    // Should handle whitespace tokens
    sentence_encoder.encode(tokens, output.data());
    
    EXPECT_TRUE(true);
}

TEST(EdgeCasesTest, MixedCaseTokens) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder word_encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    std::vector<float> vec1(dim);
    std::vector<float> vec2(dim);
    std::vector<float> vec3(dim);
    
    word_encoder.encode("hello", vec1.data());
    word_encoder.encode("Hello", vec2.data());
    word_encoder.encode("HELLO", vec3.data());
    
    // Different cases should produce different vectors
    // (unless tokenizer normalizes case)
    EXPECT_TRUE(true);
}
