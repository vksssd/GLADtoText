#include "mean_sentence_encoder.h"
#include "word_encoder.h"
#include <cstring>

MeanSentenceEncoder::MeanSentenceEncoder(
    const WordEncoder& word_encoder)
    : word_encoder_(word_encoder), 
      dim_(word_encoder.dim())
{
    scratch_word_.resize(dim_);
}

void MeanSentenceEncoder::encode(
    const std::vector<std::string>& tokens, 
    float* out) const 
{
    std::memset(out, 0, dim_ * sizeof(float));

    if (tokens.empty()) return;

    // Encode each word and accumulate
    for (const auto& token : tokens) {
        word_encoder_.encode(token, scratch_word_.data());

        for (int j = 0; j < dim_; ++j) {
            out[j] += scratch_word_[j];
        }
    }

    // Average
    float inv = 1.0f / tokens.size();
    for (int j = 0; j < dim_; ++j) {
        out[j] *= inv;
    }
}