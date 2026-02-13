#pragma once

#include <string>
#include <vector>

class WordEncoder;

class MeanSentenceEncoder {
public:
    explicit MeanSentenceEncoder(const WordEncoder& word_encoder);

    void encode(const std::vector<std::string>& tokens, float* out) const;
    
    int dim() const { return dim_; }

private:
    const WordEncoder& word_encoder_;
    int dim_;

    mutable std::vector<float> scratch_word_;
};