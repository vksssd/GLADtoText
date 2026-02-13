#include <gtest/gtest.h>
#include "training/simple_trainer.h"
#include "classifier/linear_classifier.h"
#include "encoder/mean_sentence_encoder.h"
#include "tokenizer/english_tokenizer.h"
#include "embedding/embedding_table.h"
#include "encoder/word_encoder.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"
#include "loss/softmax.h"

TEST(TrainingTest, OverfitTinyDataset) {

    // Minimal configuration
    int dim = 32;
    int buckets = 10000;

    EmbeddingTable embedding(buckets, dim, 42);
    NGramGenerator ngram(3, 6);
    PhoneticEncoder phonetic;

    WordEncoder word_encoder(
        embedding,
        ngram,
        &phonetic,
        buckets,
        0.2f);

    MeanSentenceEncoder sentence_encoder(word_encoder);

    LinearClassifier classifier(dim, 2, 42);

    EnglishTokenizer tokenizer;

    SimpleTrainer trainer(
        tokenizer,
        sentence_encoder,
        classifier,
        dim,
        2);

    std::vector<Sample> data = {
        {"good movie", 1},
        {"bad movie", 0},
        {"good good", 1},
        {"bad bad", 0}
    };

    for (int epoch = 0; epoch < 200; ++epoch)
        trainer.train_epoch(data, 0.1f);

    int correct = 0;

    std::vector<float> sentence(dim);
    std::vector<float> logits(2);

    for (auto& s : data) {

        auto tokens = tokenizer.tokenize(s.text);

        sentence_encoder.encode(tokens,
                                sentence.data());

        classifier.forward(sentence.data(),
                           logits.data());

        softmax(logits.data(), 2);

        int pred =
            logits[0] > logits[1] ? 0 : 1;

        if (pred == s.label)
            correct++;
    }

    EXPECT_EQ(correct, data.size());
}