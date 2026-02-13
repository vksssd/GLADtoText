#pragma once

#include <vector>
#include <string>

struct Sample {
    std::string text;
    int label;
};

class EnglishTokenizer;
class MeanSentenceEncoder;
class LinearClassifier;

class SimpleTrainer {
public:
    SimpleTrainer(EnglishTokenizer& tokenizer,
                  MeanSentenceEncoder& encoder,
                  LinearClassifier& classifier,
                  int input_dim,
                  int num_classes);

    float train_epoch(const std::vector<Sample>& data,
                      float learning_rate);

private:
    EnglishTokenizer& tokenizer_;
    MeanSentenceEncoder& encoder_;
    LinearClassifier& classifier_;

    int dim_;
    int num_classes_;

    std::vector<float> sentence_;
    std::vector<float> logits_;
    std::vector<float> dlogits_;
};