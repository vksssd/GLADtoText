#include <gtest/gtest.h>
#include "training/simple_trainer.h"
#include "classifier/linear_classifier.h"
#include "encoder/mean_sentence_encoder.h"
#include "encoder/word_encoder.h"
#include "tokenizer/english_tokenizer.h"
#include "embedding/embedding_table.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"
#include "loss/softmax.h"

// Integration tests for full pipeline

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        dim = 64;
        buckets = 10000;
        num_classes = 3;
        
        embedding = std::make_unique<EmbeddingTable>(buckets, dim, 42);
        ngram = std::make_unique<NGramGenerator>(3, 6);
        phonetic = std::make_unique<PhoneticEncoder>();
        
        word_encoder = std::make_unique<WordEncoder>(
            *embedding, *ngram, phonetic.get(), buckets, 0.2f);
        
        sentence_encoder = std::make_unique<MeanSentenceEncoder>(*word_encoder);
        
        classifier = std::make_unique<LinearClassifier>(dim, num_classes, 42);
        
        tokenizer = std::make_unique<EnglishTokenizer>();
        
        trainer = std::make_unique<SimpleTrainer>(
            *tokenizer, *sentence_encoder, *classifier, dim, num_classes);
    }
    
    int dim;
    int buckets;
    int num_classes;
    
    std::unique_ptr<EmbeddingTable> embedding;
    std::unique_ptr<NGramGenerator> ngram;
    std::unique_ptr<PhoneticEncoder> phonetic;
    std::unique_ptr<WordEncoder> word_encoder;
    std::unique_ptr<MeanSentenceEncoder> sentence_encoder;
    std::unique_ptr<LinearClassifier> classifier;
    std::unique_ptr<EnglishTokenizer> tokenizer;
    std::unique_ptr<SimpleTrainer> trainer;
};

TEST_F(IntegrationTest, EndToEndPrediction) {
    // Test full pipeline: text -> tokens -> encoding -> classification
    
    std::string text = "hello world";
    auto tokens = tokenizer->tokenize(text);
    
    std::vector<float> sentence_vec(dim);
    sentence_encoder->encode(tokens, sentence_vec.data());
    
    std::vector<float> logits(num_classes);
    classifier->forward(sentence_vec.data(), logits.data());
    
    softmax(logits.data(), num_classes);
    
    // Check probabilities sum to 1
    float sum = 0.0f;
    for (float p : logits)
        sum += p;
    
    EXPECT_NEAR(sum, 1.0f, 1e-5f);
}

TEST_F(IntegrationTest, TrainingReducesLoss) {
    std::vector<Sample> data = {
        {"good movie", 0},
        {"bad film", 1},
        {"excellent show", 0},
        {"terrible movie", 1},
        {"great film", 0},
        {"awful show", 1}
    };
    
    float initial_loss = trainer->train_epoch(data, 0.1f);
    
    // Train for more epochs
    float final_loss = initial_loss;
    for (int i = 0; i < 50; ++i) {
        final_loss = trainer->train_epoch(data, 0.1f);
    }
    
    // Loss should decrease
    EXPECT_LT(final_loss, initial_loss);
}

TEST_F(IntegrationTest, ConsistentPredictionAfterTraining) {
    std::vector<Sample> data = {
        {"positive sentiment", 0},
        {"negative sentiment", 1}
    };
    
    // Train
    for (int i = 0; i < 20; ++i) {
        trainer->train_epoch(data, 0.1f);
    }
    
    // Make prediction twice
    auto tokens = tokenizer->tokenize("positive sentiment");
    
    std::vector<float> sentence_vec(dim);
    std::vector<float> logits1(num_classes);
    std::vector<float> logits2(num_classes);
    
    sentence_encoder->encode(tokens, sentence_vec.data());
    classifier->forward(sentence_vec.data(), logits1.data());
    
    sentence_encoder->encode(tokens, sentence_vec.data());
    classifier->forward(sentence_vec.data(), logits2.data());
    
    // Predictions should be identical
    for (int i = 0; i < num_classes; ++i)
        EXPECT_FLOAT_EQ(logits1[i], logits2[i]);
}

TEST_F(IntegrationTest, MultipleEpochsConvergence) {
    std::vector<Sample> data = {
        {"class zero", 0},
        {"class one", 1},
        {"class two", 2}
    };
    
    std::vector<float> losses;
    
    for (int epoch = 0; epoch < 100; ++epoch) {
        float loss = trainer->train_epoch(data, 0.05f);
        losses.push_back(loss);
    }
    
    // Loss should generally decrease (allowing some noise)
    EXPECT_LT(losses.back(), losses.front());
}

TEST_F(IntegrationTest, DifferentTextsProduceDifferentPredictions) {
    std::string text1 = "hello world";
    std::string text2 = "goodbye universe";
    
    auto tokens1 = tokenizer->tokenize(text1);
    auto tokens2 = tokenizer->tokenize(text2);
    
    std::vector<float> sentence_vec(dim);
    std::vector<float> logits1(num_classes);
    std::vector<float> logits2(num_classes);
    
    sentence_encoder->encode(tokens1, sentence_vec.data());
    classifier->forward(sentence_vec.data(), logits1.data());
    
    sentence_encoder->encode(tokens2, sentence_vec.data());
    classifier->forward(sentence_vec.data(), logits2.data());
    
    // Logits should be different
    bool different = false;
    for (int i = 0; i < num_classes; ++i) {
        if (std::abs(logits1[i] - logits2[i]) > 1e-6f) {
            different = true;
            break;
        }
    }
    
    EXPECT_TRUE(different);
}

TEST_F(IntegrationTest, EmptyTextHandling) {
    std::string text = "";
    auto tokens = tokenizer->tokenize(text);
    
    std::vector<float> sentence_vec(dim);
    sentence_encoder->encode(tokens, sentence_vec.data());
    
    std::vector<float> logits(num_classes);
    classifier->forward(sentence_vec.data(), logits.data());
    
    // Should handle empty text gracefully
    for (float v : logits)
        EXPECT_FALSE(std::isnan(v));
}

TEST_F(IntegrationTest, LongTextHandling) {
    // Create a very long text
    std::string text;
    for (int i = 0; i < 1000; ++i)
        text += "word" + std::to_string(i) + " ";
    
    auto tokens = tokenizer->tokenize(text);
    
    std::vector<float> sentence_vec(dim);
    sentence_encoder->encode(tokens, sentence_vec.data());
    
    std::vector<float> logits(num_classes);
    classifier->forward(sentence_vec.data(), logits.data());
    
    // Should handle long text without crash
    EXPECT_TRUE(true);
}

TEST_F(IntegrationTest, BatchTrainingConsistency) {
    std::vector<Sample> data = {
        {"sample one", 0},
        {"sample two", 1},
        {"sample three", 2}
    };
    
    // Train once
    float loss1 = trainer->train_epoch(data, 0.1f);
    
    // Reset and train again with same seed
    embedding = std::make_unique<EmbeddingTable>(buckets, dim, 42);
    word_encoder = std::make_unique<WordEncoder>(
        *embedding, *ngram, phonetic.get(), buckets, 0.2f);
    sentence_encoder = std::make_unique<MeanSentenceEncoder>(*word_encoder);
    classifier = std::make_unique<LinearClassifier>(dim, num_classes, 42);
    trainer = std::make_unique<SimpleTrainer>(
        *tokenizer, *sentence_encoder, *classifier, dim, num_classes);
    
    float loss2 = trainer->train_epoch(data, 0.1f);
    
    // Losses should be identical with same seed
    EXPECT_FLOAT_EQ(loss1, loss2);
}

TEST_F(IntegrationTest, PredictionAfterNoTraining) {
    // Test prediction with untrained model
    std::string text = "test sentence";
    auto tokens = tokenizer->tokenize(text);
    
    std::vector<float> sentence_vec(dim);
    sentence_encoder->encode(tokens, sentence_vec.data());
    
    std::vector<float> logits(num_classes);
    classifier->forward(sentence_vec.data(), logits.data());
    
    softmax(logits.data(), num_classes);
    
    // Should produce valid probabilities even without training
    float sum = 0.0f;
    for (float p : logits) {
        EXPECT_GE(p, 0.0f);
        EXPECT_LE(p, 1.0f);
        sum += p;
    }
    
    EXPECT_NEAR(sum, 1.0f, 1e-5f);
}

TEST_F(IntegrationTest, SpecialCharactersInText) {
    std::string text = "Hello! How are you? I'm fine, thanks.";
    auto tokens = tokenizer->tokenize(text);
    
    std::vector<float> sentence_vec(dim);
    sentence_encoder->encode(tokens, sentence_vec.data());
    
    std::vector<float> logits(num_classes);
    classifier->forward(sentence_vec.data(), logits.data());
    
    // Should handle punctuation gracefully
    for (float v : logits)
        EXPECT_FALSE(std::isnan(v));
}

TEST_F(IntegrationTest, NumbersInText) {
    std::string text = "I have 123 apples and 456 oranges";
    auto tokens = tokenizer->tokenize(text);
    
    std::vector<float> sentence_vec(dim);
    sentence_encoder->encode(tokens, sentence_vec.data());
    
    std::vector<float> logits(num_classes);
    classifier->forward(sentence_vec.data(), logits.data());
    
    // Should handle numbers in text
    for (float v : logits)
        EXPECT_FALSE(std::isnan(v));
}
