#include <gtest/gtest.h>
#include "training/simple_trainer.h"
#include "classifier/linear_classifier.h"
#include "encoder/mean_sentence_encoder.h"
#include "tokenizer/english_tokenizer.h"
#include "embedding/embedding_table.h"
#include "encoder/word_encoder.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"


TEST(TrainingTest, DeterministicTraining) {

    int dim = 16;
    int buckets = 5000;

    EmbeddingTable embedding1(buckets, dim, 123);
    EmbeddingTable embedding2(buckets, dim, 123);

    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;

    WordEncoder word_encoder1(
        embedding1, ngram, &phonetic, buckets, 0.2f);

    WordEncoder word_encoder2(
        embedding2, ngram, &phonetic, buckets, 0.2f);

    MeanSentenceEncoder encoder1(word_encoder1);
    MeanSentenceEncoder encoder2(word_encoder2);

    LinearClassifier clf1(dim, 2, 123);
    LinearClassifier clf2(dim, 2, 123);

    EnglishTokenizer tokenizer;

    SimpleTrainer trainer1(
        tokenizer, encoder1, clf1, dim, 2);

    SimpleTrainer trainer2(
        tokenizer, encoder2, clf2, dim, 2);

    std::vector<Sample> data = {
        {"hello world", 0},
        {"good day", 1}
    };

    trainer1.train_epoch(data, 0.05f);
    trainer2.train_epoch(data, 0.05f);

    std::vector<float> sentence(dim);
    std::vector<float> logits1(2), logits2(2);

    auto tokens = tokenizer.tokenize("hello world");

    encoder1.encode(tokens, sentence.data());
    clf1.forward(sentence.data(), logits1.data());

    encoder2.encode(tokens, sentence.data());
    clf2.forward(sentence.data(), logits2.data());

    for (int i = 0; i < 2; ++i)
        EXPECT_FLOAT_EQ(logits1[i], logits2[i]);
}