#include "word_encoder.h"
#include "hashing/hash_function.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"
#include "embedding/embedding_table.h"
#include <cstring>

WordEncoder::WordEncoder(
    const EmbeddingTable& embedding,
    const NGramGenerator& ngram,
    const PhoneticEncoder* phonetic,
    int bucket_count,
    float phonetic_gamma)
    : embedding_(embedding),
      ngram_(ngram),
      phonetic_(phonetic),
      bucket_count_(bucket_count),
      gamma_(phonetic_gamma)
{
    scratch_ngrams_.reserve(32);
    scratch_wrapped_.reserve(64);
    scratch_phonetic_.reserve(8);
}

int WordEncoder::dim() const {
    return embedding_.dim();
}

void WordEncoder::encode(
    const std::string& token,
    float* out) const
{
    int dim = embedding_.dim();

    std::memset(out, 0, dim * sizeof(float));

    scratch_ngrams_.clear();
    scratch_wrapped_.clear();

    ngram_.generate(token,
                    scratch_wrapped_,
                    scratch_ngrams_);

    int count = 0;

    for (auto& g : scratch_ngrams_) {
        uint64_t hash = HashFunction::fnv1a(g);
        int bucket = hash % bucket_count_;

        const float* row = embedding_.row(bucket);

        for (int j = 0; j < dim; ++j)
            out[j] += row[j];

        count++;
    }

    if (count > 0) {
        float inv = 1.0f / count;
        for (int j = 0; j < dim; ++j)
            out[j] *= inv;
    }

    if (phonetic_ && gamma_ > 0.0f) {
        scratch_phonetic_.clear();
        scratch_phonetic_ =
            phonetic_->encode(token);

        if (!scratch_phonetic_.empty()) {
            uint64_t hash =
                HashFunction::fnv1a(scratch_phonetic_);

            int bucket = hash % bucket_count_;

            const float* row =
                embedding_.row(bucket);

            for (int j = 0; j < dim; ++j)
                out[j] += gamma_ * row[j];
        }
    }
}
