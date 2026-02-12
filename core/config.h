#pragma once

// Global configuration for the model
struct Config {
    // Embedding dimensions
    int dim = 100;
    
    // Character n-gram settings
    int char_ngram_min = 3;
    int char_ngram_max = 6;
    int bucket_size = 2000000;
    
    // Training hyperparameters
    float learning_rate = 0.05f;
    int epochs = 10;
    int negative_samples = 5;
    int context_window = 5;
    int min_word_count = 5;
    
    // Layer activation flags
    bool use_char_ngrams = true;
    bool use_grammar_units = true;
    bool use_phonetic = true;
    bool use_vector_attention = true;
    bool use_sentence_encoder = false;
    
    // Memory optimization
    bool use_sparse_matrices = false;
    bool use_quantization = false;
    float pruning_threshold = 1e-4f;
    
    // Regularization
    float l2_lambda = 0.0f;  // L2 regularization strength
    float dropout_rate = 0.0f;  // Dropout rate (0 = no dropout)
    float gradient_clip = 5.0f;  // Gradient clipping threshold
    
    // Advanced settings
    bool normalize_embeddings = false;
    bool use_xavier_init = false;
    bool use_he_init = false;
    int random_seed = 42;
    
    // Validation
    bool validate() const {
        if (dim <= 0 || dim > 1000) return false;
        if (char_ngram_min < 1 || char_ngram_max > 10) return false;
        if (char_ngram_min > char_ngram_max) return false;
        if (learning_rate <= 0.0f || learning_rate > 1.0f) return false;
        if (epochs <= 0) return false;
        if (l2_lambda < 0.0f) return false;
        if (dropout_rate < 0.0f || dropout_rate >= 1.0f) return false;
        return true;
    }
};
