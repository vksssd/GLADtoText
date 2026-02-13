#include "training/simple_trainer.h"
#include "loss/softmax.h"
#include "tokenizer/english_tokenizer.h"
#include "encoder/mean_sentence_encoder.h"
#include "classifier/linear_classifier.h"

SimpleTrainer::SimpleTrainer(
    EnglishTokenizer& tokenizer,
    MeanSentenceEncoder& encoder,
    LinearClassifier& classifier,
    int input_dim,
    int num_classes)
    : tokenizer_(tokenizer),
      encoder_(encoder),
      classifier_(classifier),
      dim_(input_dim),
      num_classes_(num_classes),
      sentence_(input_dim),
      logits_(num_classes),
      dlogits_(num_classes)
{}

float SimpleTrainer::train_epoch(
    const std::vector<Sample>& data,
    float learning_rate)
{
    float total_loss = 0.0f;

    for (const auto& sample : data) {

        auto tokens =
            tokenizer_.tokenize(sample.text);

        encoder_.encode(tokens,
                        sentence_.data());

        classifier_.forward(sentence_.data(),
                            logits_.data());

        softmax(logits_.data(),
                num_classes_);

        float loss =
            cross_entropy(logits_.data(),
                          sample.label);

        total_loss += loss;

        // dlogits = probs
        for (int i = 0; i < num_classes_; ++i)
            dlogits_[i] = logits_[i];

        dlogits_[sample.label] -= 1.0f;

        classifier_.backward_sgd(
            sentence_.data(),
            dlogits_.data(),
            nullptr,
            learning_rate);
    }

    return total_loss / data.size();
}