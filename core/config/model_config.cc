#include "model_config.h"

bool ModelConfig::operator==(const ModelConfig& other) const {
    return bucket_count == other.bucket_count &&
           embedding_dim == other.embedding_dim &&
           ngram_min == other.ngram_min &&
           ngram_max == other.ngram_max &&
           phonetic_gamma == other.phonetic_gamma &&
           num_heads == other.num_heads &&
           use_projection == other.use_projection &&
           use_residual == other.use_residual &&
           epochs == other.epochs &&
           batch_size == other.batch_size &&
           learning_rate_adam == other.learning_rate_adam &&
           learning_rate_sgd == other.learning_rate_sgd &&
           weight_decay == other.weight_decay &&
           seed == other.seed;
}

void ModelConfig::validate() const {
    if(bucket_count<=0)
        throw std::invalid_argument("bucket_count must be >0");
    if(embedding_dim<=0)
        throw std::invalid_argument("embedding_dim must be > 0");
    if(ngram_min<=0 || ngram_max < ngram_min)
        throw std::invalid_argument("Invalid ngram range");
    if(num_heads<=0)
        throw std::invalid_argument("num_heads must be > 0");
    if(phonetic_gamma<0.0f)
        throw std::invalid_argument("phonetic_gamma must be >= 0");
    
}