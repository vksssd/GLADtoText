#pragma once

#include<cstdint>
#include<stdexcept>

enum class ProjectionMode {
    DENSE,
    LOWRANK,
    SPARSE,
    HYBRID
};

enum class PrecisionMode {
    FP32,
    FP16
};

struct ModelConfig {

    // embeddings
    int bucket_count = 200000;
    int embedding_dim = 256;
    int ngram_min = 3;
    int ngram_max = 6;

    // phonetic
    bool use_phonetic = true;
    float phonetic_gamma = 0.2f;

    // encoder
    int num_heads = 4;
    bool use_projection = true;
    bool use_residual = true;

    // training
    int epochs = 10;
    int batch_size = 64;
    float learning_rate_adam = 1e-3f;
    float learning_rate_sgd = 1e-2f;
    float weight_decay = 1e-4f;

    // deterministic seeds
    ProjectionMode projection_mode = ProjectionMode::DENSE;
    PrecisionMode precision_mode = PrecisionMode::FP32;
    uint64_t seed = 42;

    void validate() const;
    
    bool operator == (const ModelConfig& other) const;
};