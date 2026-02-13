#include <gtest/gtest.h>
#include "encoder/word_encoder.h"
#include "embedding/embedding_table.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"
#include <cmath>

TEST(WordEncoderTest, BasicEncoding) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    std::vector<float> output(dim);
    encoder.encode("hello", output.data());
    
    // Check output is not all zeros
    float sum = 0.0f;
    for (float v : output)
        sum += std::abs(v);
    
    EXPECT_GT(sum, 0.0f);
}

TEST(WordEncoderTest, DeterministicEncoding) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 123);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    std::vector<float> output1(dim);
    std::vector<float> output2(dim);
    
    encoder.encode("world", output1.data());
    encoder.encode("world", output2.data());
    
    for (int i = 0; i < dim; ++i)
        EXPECT_FLOAT_EQ(output1[i], output2[i]);
}

TEST(WordEncoderTest, DifferentWordsDifferentVectors) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    std::vector<float> vec1(dim);
    std::vector<float> vec2(dim);
    
    encoder.encode("hello", vec1.data());
    encoder.encode("world", vec2.data());
    
    // Vectors should be different
    bool different = false;
    for (int i = 0; i < dim; ++i) {
        if (std::abs(vec1[i] - vec2[i]) > 1e-6f) {
            different = true;
            break;
        }
    }
    
    EXPECT_TRUE(different);
}

TEST(WordEncoderTest, EmptyStringHandling) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    std::vector<float> output(dim);
    encoder.encode("", output.data());
    
    // Empty string should produce zero vector or handle gracefully
    // (depends on implementation - just ensure no crash)
    EXPECT_TRUE(true);
}

TEST(WordEncoderTest, PhoneticContribution) {
    int dim = 32;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    // With phonetic
    WordEncoder encoder_with(embedding, ngram, &phonetic, buckets, 0.5f);
    
    // Without phonetic
    WordEncoder encoder_without(embedding, ngram, nullptr, buckets, 0.0f);
    
    std::vector<float> vec_with(dim);
    std::vector<float> vec_without(dim);
    
    encoder_with.encode("hello", vec_with.data());
    encoder_without.encode("hello", vec_without.data());
    
    // Vectors should be different when phonetic is used
    bool different = false;
    for (int i = 0; i < dim; ++i) {
        if (std::abs(vec_with[i] - vec_without[i]) > 1e-6f) {
            different = true;
            break;
        }
    }
    
    EXPECT_TRUE(different);
}

TEST(WordEncoderTest, SimilarWordsSimilarVectors) {
    int dim = 64;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder encoder(embedding, ngram, &phonetic, buckets, 0.3f);
    
    std::vector<float> vec1(dim);
    std::vector<float> vec2(dim);
    std::vector<float> vec3(dim);
    
    encoder.encode("running", vec1.data());
    encoder.encode("runner", vec2.data());
    encoder.encode("elephant", vec3.data());
    
    // Compute cosine similarity
    auto cosine = [](const std::vector<float>& a, const std::vector<float>& b) {
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
    };
    
    float sim_similar = cosine(vec1, vec2);
    float sim_different = cosine(vec1, vec3);
    
    // Similar words should have higher similarity
    EXPECT_GT(sim_similar, sim_different);
}

TEST(WordEncoderTest, DimensionAccessor) {
    int dim = 128;
    int buckets = 10000;
    
    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;
    
    WordEncoder encoder(embedding, ngram, &phonetic, buckets, 0.2f);
    
    EXPECT_EQ(encoder.dim(), dim);
}
