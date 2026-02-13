#include "embedding_table.h"
#include "utils/aligned_alloc.h"
#include "utils/rng.h"
#include <cmath>
#include <cassert>

EmbeddingTable::EmbeddingTable(
    int bucket_count,
    int dim,
    uint64_t seed)
    : bucket_count_(bucket_count),
      dim_(dim),
      data_(nullptr)
{
    size_t total = static_cast<size_t>(bucket_count_) *
                   dim_ * sizeof(float);

    data_ = static_cast<float*>(
        aligned_malloc(total, 32)
    );

    if (!data_)
        throw std::bad_alloc();

    RNG rng(seed);

    float bound = std::sqrt(1.0f / dim_);

    for (int i = 0; i < bucket_count_; ++i) {
        float* r = row(i);
        for (int j = 0; j < dim_; ++j)
            r[j] = rng.uniform(-bound, bound);
    }
}

EmbeddingTable::~EmbeddingTable() {
    if (data_)
        aligned_free(data_);
}

float* EmbeddingTable::row(int bucket) {
#ifndef NDEBUG
    assert(bucket >= 0 && bucket < bucket_count_);
#endif
    return data_ + static_cast<size_t>(bucket) * dim_;
}

const float* EmbeddingTable::row(int bucket) const {
#ifndef NDEBUG
    assert(bucket >= 0 && bucket < bucket_count_);
#endif
    return data_ + static_cast<size_t>(bucket) * dim_;
}

int EmbeddingTable::bucket_count() const noexcept {
    return bucket_count_;
}

int EmbeddingTable::dim() const noexcept {
    return dim_;
}

size_t EmbeddingTable::memory_bytes() const noexcept {
    return static_cast<size_t>(bucket_count_) *
           dim_ * sizeof(float);
}
