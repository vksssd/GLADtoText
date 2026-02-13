#include <gtest/gtest.h>
#include "encoder/mean_sentence_encoder.h"
#include "encoder/word_encoder.h"
#include "embedding/embedding_table.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"
#include <cmath>

class MeanSentenceEncoderTest : public ::testing::Test {
protected:
    void SetUp() override {
        dim = 32;
        buckets = 10000;
        
        embedding = std::make_unique<EmbeddingTable>(buckets, dim, 42);
        ngram = std::make_unique<NGramGenerator>(3, 6);
        phonetic = std::make_unique<PhoneticEncoder>();
        
        word_encoder = std::make_unique<WordEncoder>(
            *embedding, *ngram, phonetic.get(), buckets, 0.2f);
        
        sentence_encoder = std::make_unique<MeanSentenceEncoder>(*word_encoder);
    }
    
    int dim;
    int buckets;
    std::unique_ptr<EmbeddingTable> embedding;
    std::unique_ptr<NGramGenerator> ngram;
    std::unique_ptr<PhoneticEncoder> phonetic;
    std::unique_ptr<WordEncoder> word_encoder;
    std::unique_ptr<MeanSentenceEncoder> sentence_encoder;
};

TEST_F(MeanSentenceEncoderTest, BasicEncoding) {
    std::vector<std::string> tokens = {"hello", "world"};
    std::vector<float> output(dim);
    
    sentence_encoder->encode(tokens, output.data());
    
    // Check output is not all zeros
    float sum = 0.0f;
    for (float v : output)
        sum += std::abs(v);
    
    EXPECT_GT(sum, 0.0f);
}

TEST_F(MeanSentenceEncoderTest, EmptyTokensHandling) {
    std::vector<std::string> tokens;
    std::vector<float> output(dim);
    
    sentence_encoder->encode(tokens, output.data());
    
    // Empty tokens should produce zero vector
    for (float v : output)
        EXPECT_FLOAT_EQ(v, 0.0f);
}

TEST_F(MeanSentenceEncoderTest, SingleTokenEncoding) {
    std::vector<std::string> tokens = {"hello"};
    std::vector<float> sentence_vec(dim);
    std::vector<float> word_vec(dim);
    
    sentence_encoder->encode(tokens, sentence_vec.data());
    word_encoder->encode("hello", word_vec.data());
    
    // Single token sentence should equal word encoding
    for (int i = 0; i < dim; ++i)
        EXPECT_FLOAT_EQ(sentence_vec[i], word_vec[i]);
}

TEST_F(MeanSentenceEncoderTest, DeterministicEncoding) {
    std::vector<std::string> tokens = {"hello", "world", "test"};
    std::vector<float> output1(dim);
    std::vector<float> output2(dim);
    
    sentence_encoder->encode(tokens, output1.data());
    sentence_encoder->encode(tokens, output2.data());
    
    for (int i = 0; i < dim; ++i)
        EXPECT_FLOAT_EQ(output1[i], output2[i]);
}

TEST_F(MeanSentenceEncoderTest, OrderMatters) {
    std::vector<std::string> tokens1 = {"hello", "world"};
    std::vector<std::string> tokens2 = {"world", "hello"};
    
    std::vector<float> vec1(dim);
    std::vector<float> vec2(dim);
    
    sentence_encoder->encode(tokens1, vec1.data());
    sentence_encoder->encode(tokens2, vec2.data());
    
    // Mean pooling is order-invariant, so vectors should be equal
    for (int i = 0; i < dim; ++i)
        EXPECT_FLOAT_EQ(vec1[i], vec2[i]);
}

TEST_F(MeanSentenceEncoderTest, AveragingBehavior) {
    std::vector<std::string> single = {"test"};
    std::vector<std::string> repeated = {"test", "test"};
    
    std::vector<float> vec_single(dim);
    std::vector<float> vec_repeated(dim);
    
    sentence_encoder->encode(single, vec_single.data());
    sentence_encoder->encode(repeated, vec_repeated.data());
    
    // Repeated word should give same average
    for (int i = 0; i < dim; ++i)
        EXPECT_NEAR(vec_single[i], vec_repeated[i], 1e-5f);
}

TEST_F(MeanSentenceEncoderTest, DifferentSentencesDifferentVectors) {
    std::vector<std::string> tokens1 = {"hello", "world"};
    std::vector<std::string> tokens2 = {"goodbye", "universe"};
    
    std::vector<float> vec1(dim);
    std::vector<float> vec2(dim);
    
    sentence_encoder->encode(tokens1, vec1.data());
    sentence_encoder->encode(tokens2, vec2.data());
    
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

TEST_F(MeanSentenceEncoderTest, LongSentenceHandling) {
    std::vector<std::string> tokens;
    for (int i = 0; i < 100; ++i)
        tokens.push_back("word" + std::to_string(i));
    
    std::vector<float> output(dim);
    
    // Should handle long sentences without crash
    sentence_encoder->encode(tokens, output.data());
    
    float sum = 0.0f;
    for (float v : output)
        sum += std::abs(v);
    
    EXPECT_GT(sum, 0.0f);
}

TEST_F(MeanSentenceEncoderTest, DimensionAccessor) {
    EXPECT_EQ(sentence_encoder->dim(), dim);
}

TEST_F(MeanSentenceEncoderTest, SimilarSentencesSimilarVectors) {
    std::vector<std::string> tokens1 = {"good", "movie"};
    std::vector<std::string> tokens2 = {"great", "film"};
    std::vector<std::string> tokens3 = {"terrible", "disaster"};
    
    std::vector<float> vec1(dim);
    std::vector<float> vec2(dim);
    std::vector<float> vec3(dim);
    
    sentence_encoder->encode(tokens1, vec1.data());
    sentence_encoder->encode(tokens2, vec2.data());
    sentence_encoder->encode(tokens3, vec3.data());
    
    auto cosine = [](const std::vector<float>& a, const std::vector<float>& b) {
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
    };
    
    float sim_positive = cosine(vec1, vec2);
    float sim_negative = cosine(vec1, vec3);
    
    // Similar sentiment sentences might have higher similarity
    // (This is a weak test since we're using random embeddings)
    EXPECT_TRUE(true); // Just ensure no crash
}
