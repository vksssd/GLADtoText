#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

// Model configuration for controlling which layers are active
struct ModelConfig {
    // Layer activation flags
    bool use_word_embeddings = true;      // Base word embeddings
    bool use_char_ngrams = true;          // Character n-grams
    bool use_grammar_units = true;        // Grammar units
    bool use_phonetic = true;             // Phonetic encoding
    bool use_vector_attention = true;     // Vector attention
    bool use_sentence_encoder = false;    // Sentence-level encoding
    bool use_position_encoding = false;   // Position encoding (requires sentence_encoder)
    bool use_self_attention = false;      // Self-attention (requires sentence_encoder)
    
    // Model parameters
    int dim = 100;
    int vocab_size = 10000;
    int bucket_size = 2000000;
    int minn = 3;
    int maxn = 6;
    int num_classes = 10;
    
    // Memory optimization
    bool use_sparse_matrices = true;      // Use sparse storage
    bool use_quantization = false;        // Use int8 quantization
    float pruning_threshold = 1e-4f;      // Prune weights below this
    
    // Calculate memory usage for each component
    size_t calculateWordEmbeddingsSize() const {
        if (!use_word_embeddings) return 0;
        size_t size = vocab_size * dim * sizeof(float);
        if (use_quantization) size /= 4;  // int8 vs float32
        if (use_sparse_matrices) size /= 10;  // ~90% sparsity
        return size;
    }
    
    size_t calculateCharNgramsSize() const {
        if (!use_char_ngrams) return 0;
        size_t size = bucket_size * dim * sizeof(float);
        if (use_quantization) size /= 4;
        if (use_sparse_matrices) size /= 10;
        return size;
    }
    
    size_t calculateGrammarSize() const {
        if (!use_grammar_units) return 0;
        // Grammar units: ~100 units
        size_t size = 100 * dim * sizeof(float);
        if (use_quantization) size /= 4;
        if (use_sparse_matrices) size /= 10;
        return size;
    }
    
    size_t calculatePhoneticSize() const {
        if (!use_phonetic) return 0;
        // Phonetic: ~50 phonemes
        size_t size = 50 * dim * sizeof(float);
        if (use_quantization) size /= 4;
        if (use_sparse_matrices) size /= 10;
        return size;
    }
    
    size_t calculateVectorAttentionSize() const {
        if (!use_vector_attention) return 0;
        // Attention weights: dim-dimensional vector
        size_t size = dim * sizeof(float);
        return size;
    }
    
    size_t calculateSentenceEncoderSize() const {
        if (!use_sentence_encoder) return 0;
        size_t size = 0;
        
        // Position encoder: no parameters (computed on-the-fly)
        
        // Self-attention: Q, K, V matrices (dim x dim each)
        if (use_self_attention) {
            size_t attention_size = 3 * dim * dim * sizeof(float);
            if (use_quantization) attention_size /= 4;
            if (use_sparse_matrices) attention_size /= 10;
            size += attention_size;
        }
        
        return size;
    }
    
    size_t calculateClassifierSize() const {
        // Classifier prototypes: num_classes x dim
        size_t size = num_classes * dim * sizeof(float);
        if (use_quantization) size /= 4;
        return size;
    }
    
    size_t calculateTotalSize() const {
        return calculateWordEmbeddingsSize() +
               calculateCharNgramsSize() +
               calculateGrammarSize() +
               calculatePhoneticSize() +
               calculateVectorAttentionSize() +
               calculateSentenceEncoderSize() +
               calculateClassifierSize();
    }
    
    // Load config from file
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Cannot open config file: " << filename << "\n";
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') continue;
            
            // Parse key=value
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;
            
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Set config values
            if (key == "use_word_embeddings") use_word_embeddings = (value == "true" || value == "1");
            else if (key == "use_char_ngrams") use_char_ngrams = (value == "true" || value == "1");
            else if (key == "use_grammar_units") use_grammar_units = (value == "true" || value == "1");
            else if (key == "use_phonetic") use_phonetic = (value == "true" || value == "1");
            else if (key == "use_vector_attention") use_vector_attention = (value == "true" || value == "1");
            else if (key == "use_sentence_encoder") use_sentence_encoder = (value == "true" || value == "1");
            else if (key == "use_position_encoding") use_position_encoding = (value == "true" || value == "1");
            else if (key == "use_self_attention") use_self_attention = (value == "true" || value == "1");
            else if (key == "dim") dim = std::stoi(value);
            else if (key == "vocab_size") vocab_size = std::stoi(value);
            else if (key == "bucket_size") bucket_size = std::stoi(value);
            else if (key == "minn") minn = std::stoi(value);
            else if (key == "maxn") maxn = std::stoi(value);
            else if (key == "num_classes") num_classes = std::stoi(value);
            else if (key == "use_sparse_matrices") use_sparse_matrices = (value == "true" || value == "1");
            else if (key == "use_quantization") use_quantization = (value == "true" || value == "1");
            else if (key == "pruning_threshold") pruning_threshold = std::stof(value);
        }
        
        // Validate dependencies
        if (use_sentence_encoder) {
            use_position_encoding = true;
            use_self_attention = true;
        }
        
        return true;
    }
    
    // Save config to file
    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Cannot write config file: " << filename << "\n";
            return false;
        }
        
        file << "# GLADtoTEXT Model Configuration\n";
        file << "# Generated configuration file\n\n";
        
        file << "# Layer Activation\n";
        file << "use_word_embeddings=" << (use_word_embeddings ? "true" : "false") << "\n";
        file << "use_char_ngrams=" << (use_char_ngrams ? "true" : "false") << "\n";
        file << "use_grammar_units=" << (use_grammar_units ? "true" : "false") << "\n";
        file << "use_phonetic=" << (use_phonetic ? "true" : "false") << "\n";
        file << "use_vector_attention=" << (use_vector_attention ? "true" : "false") << "\n";
        file << "use_sentence_encoder=" << (use_sentence_encoder ? "true" : "false") << "\n";
        file << "use_position_encoding=" << (use_position_encoding ? "true" : "false") << "\n";
        file << "use_self_attention=" << (use_self_attention ? "true" : "false") << "\n\n";
        
        file << "# Model Parameters\n";
        file << "dim=" << dim << "\n";
        file << "vocab_size=" << vocab_size << "\n";
        file << "bucket_size=" << bucket_size << "\n";
        file << "minn=" << minn << "\n";
        file << "maxn=" << maxn << "\n";
        file << "num_classes=" << num_classes << "\n\n";
        
        file << "# Memory Optimization\n";
        file << "use_sparse_matrices=" << (use_sparse_matrices ? "true" : "false") << "\n";
        file << "use_quantization=" << (use_quantization ? "true" : "false") << "\n";
        file << "pruning_threshold=" << pruning_threshold << "\n";
        
        return true;
    }
    
    // Print size breakdown
    void printSizeBreakdown() const {
        std::cout << "Model Size Breakdown:\n";
        std::cout << "====================\n";
        
        auto formatSize = [](size_t bytes) -> std::string {
            if (bytes < 1024) return std::to_string(bytes) + " B";
            if (bytes < 1024 * 1024) return std::to_string(bytes / 1024) + " KB";
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        };
        
        size_t word_emb = calculateWordEmbeddingsSize();
        size_t char_ng = calculateCharNgramsSize();
        size_t grammar = calculateGrammarSize();
        size_t phonetic = calculatePhoneticSize();
        size_t vec_att = calculateVectorAttentionSize();
        size_t sent_enc = calculateSentenceEncoderSize();
        size_t classifier = calculateClassifierSize();
        size_t total = calculateTotalSize();
        
        std::cout << "Word Embeddings:     " << formatSize(word_emb) 
                  << " (" << (use_word_embeddings ? "ON" : "OFF") << ")\n";
        std::cout << "Char N-grams:        " << formatSize(char_ng) 
                  << " (" << (use_char_ngrams ? "ON" : "OFF") << ")\n";
        std::cout << "Grammar Units:       " << formatSize(grammar) 
                  << " (" << (use_grammar_units ? "ON" : "OFF") << ")\n";
        std::cout << "Phonetic:            " << formatSize(phonetic) 
                  << " (" << (use_phonetic ? "ON" : "OFF") << ")\n";
        std::cout << "Vector Attention:    " << formatSize(vec_att) 
                  << " (" << (use_vector_attention ? "ON" : "OFF") << ")\n";
        std::cout << "Sentence Encoder:    " << formatSize(sent_enc) 
                  << " (" << (use_sentence_encoder ? "ON" : "OFF") << ")\n";
        std::cout << "Classifier:          " << formatSize(classifier) << "\n";
        std::cout << "--------------------\n";
        std::cout << "TOTAL:               " << formatSize(total) << "\n";
        std::cout << "\n";
        
        std::cout << "Optimizations:\n";
        std::cout << "  Sparse matrices:   " << (use_sparse_matrices ? "ON (~90% reduction)" : "OFF") << "\n";
        std::cout << "  Quantization:      " << (use_quantization ? "ON (4x reduction)" : "OFF") << "\n";
        std::cout << "  Pruning threshold: " << pruning_threshold << "\n";
    }
    
    // Print active layers
    void printActiveLayers() const {
        std::cout << "Active Layers:\n";
        std::cout << "==============\n";
        if (use_word_embeddings) std::cout << "✓ Word Embeddings\n";
        if (use_char_ngrams) std::cout << "✓ Character N-grams\n";
        if (use_grammar_units) std::cout << "✓ Grammar Units\n";
        if (use_phonetic) std::cout << "✓ Phonetic Encoding\n";
        if (use_vector_attention) std::cout << "✓ Vector Attention\n";
        if (use_sentence_encoder) {
            std::cout << "✓ Sentence Encoder\n";
            if (use_position_encoding) std::cout << "  ✓ Position Encoding\n";
            if (use_self_attention) std::cout << "  ✓ Self-Attention\n";
        }
        std::cout << "\n";
    }
};

// Predefined configurations
namespace ModelConfigs {
    // Minimal: Only word embeddings
    inline ModelConfig minimal() {
        ModelConfig cfg;
        cfg.use_word_embeddings = true;
        cfg.use_char_ngrams = false;
        cfg.use_grammar_units = false;
        cfg.use_phonetic = false;
        cfg.use_vector_attention = false;
        cfg.use_sentence_encoder = false;
        cfg.dim = 30;
        return cfg;
    }
    
    // Compact: Word + char n-grams
    inline ModelConfig compact() {
        ModelConfig cfg;
        cfg.use_word_embeddings = true;
        cfg.use_char_ngrams = true;
        cfg.use_grammar_units = false;
        cfg.use_phonetic = false;
        cfg.use_vector_attention = false;
        cfg.use_sentence_encoder = false;
        cfg.dim = 30;
        cfg.use_sparse_matrices = true;
        return cfg;
    }
    
    // Standard: All features except sentence encoder
    inline ModelConfig standard() {
        ModelConfig cfg;
        cfg.use_word_embeddings = true;
        cfg.use_char_ngrams = true;
        cfg.use_grammar_units = true;
        cfg.use_phonetic = true;
        cfg.use_vector_attention = true;
        cfg.use_sentence_encoder = false;
        cfg.dim = 100;
        return cfg;
    }
    
    // Full: All features including sentence encoder
    inline ModelConfig full() {
        ModelConfig cfg;
        cfg.use_word_embeddings = true;
        cfg.use_char_ngrams = true;
        cfg.use_grammar_units = true;
        cfg.use_phonetic = true;
        cfg.use_vector_attention = true;
        cfg.use_sentence_encoder = true;
        cfg.use_position_encoding = true;
        cfg.use_self_attention = true;
        cfg.dim = 100;
        return cfg;
    }
    
    // Tiny: All features with sparse storage
    inline ModelConfig tiny() {
        ModelConfig cfg = full();
        cfg.dim = 30;
        cfg.use_sparse_matrices = true;
        cfg.pruning_threshold = 1e-4f;
        return cfg;
    }
}
