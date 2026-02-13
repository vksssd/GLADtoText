#pragma once

#include <string>
#include <vector>

class EmbeddingTable;
class NGramGenerator;
class PhoneticEncoder;

class WordEncoder {
public:
    WordEncoder(const EmbeddingTable& embedding,
                const NGramGenerator& ngram,
                const PhoneticEncoder* phonetic,
                int bucket_count,
                float phonetic_gamma);

    void encode(const std::string& token, float* out) const;
    
    // Accessors
    const EmbeddingTable& embedding() const { return embedding_; }
    int dim() const;

private:
    const EmbeddingTable& embedding_;
    const NGramGenerator& ngram_;
    const PhoneticEncoder* phonetic_;

    int bucket_count_;
    float gamma_;

    // Scratch buffers
    mutable std::vector<std::string_view> scratch_ngrams_;
    mutable std::string scratch_wrapped_;
    mutable std::string scratch_phonetic_;
};
