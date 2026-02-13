#pragma once

#include <cstddef>
#include <cstdint>

class EmbeddingTable {
public:
    EmbeddingTable(int bucket_count,
                   int dim,
                   uint64_t seed);

    ~EmbeddingTable();

    void initialize_uniform();

    float* row(int bucket);
    const float* row(int bucket) const;

    int bucket_count() const noexcept;
    int dim() const noexcept;

    size_t memory_bytes() const noexcept;

private:
    int bucket_count_;
    int dim_;
    float* data_;
};
