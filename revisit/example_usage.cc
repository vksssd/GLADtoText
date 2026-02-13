#include <iostream>
#include "config/model_config.h"
#include "embedding/embedding_table.h"
#include "ngram/ngram_generator.h"
#include "phonetic/phonetic_encoder.h"
#include "encoder/word_encoder.h"
#include "tokenizer/english_tokenizer.h"
#include "utils/logger.h"

int main() {
    Logger::log(LogLevel::INFO, "GLADtoTEXT Revisit - Example Usage");
    
    // Create configuration
    ModelConfig config;
    Logger::log(LogLevel::INFO, "Configuration created");
    std::cout << "  Embedding dim: " << config.embedding_dim << "\n";
    std::cout << "  Bucket count: " << config.bucket_count << "\n";
    std::cout << "  N-gram range: " << config.ngram_min << "-" << config.ngram_max << "\n";
    
    // Create embedding table
    Logger::log(LogLevel::INFO, "Creating embedding table...");
    EmbeddingTable embeddings(config.bucket_count, config.embedding_dim, config.seed);
    std::cout << "  Memory: " << embeddings.memory_bytes() / (1024.0 * 1024.0) << " MB\n";
    
    // Create n-gram generator
    NGramGenerator ngram_gen(config.ngram_min, config.ngram_max);
    
    // Create phonetic encoder
    PhoneticEncoder phonetic;
    
    // Create word encoder
    Logger::log(LogLevel::INFO, "Creating word encoder...");
    WordEncoder encoder(embeddings, ngram_gen, &phonetic, 
                       config.bucket_count, config.phonetic_gamma);
    
    // Create tokenizer
    EnglishTokenizer tokenizer;
    
    // Encode some words
    Logger::log(LogLevel::INFO, "Encoding words...");
    std::vector<std::string> test_words = {"hello", "world", "machine", "learning"};
    std::vector<float> embedding(config.embedding_dim);
    
    for (const auto& word : test_words) {
        encoder.encode(word, embedding.data());
        
        // Show first 5 dimensions
        std::cout << "  " << word << ": [";
        for (int i = 0; i < 5; ++i) {
            std::cout << embedding[i];
            if (i < 4) std::cout << ", ";
        }
        std::cout << ", ...]\n";
    }
    
    // Tokenize a sentence
    Logger::log(LogLevel::INFO, "Tokenizing sentence...");
    std::string sentence = "Hello, world! This is machine learning.";
    std::vector<std::string> tokens;
    tokenizer.tokenize(sentence, tokens);
    
    std::cout << "  Input: \"" << sentence << "\"\n";
    std::cout << "  Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    std::cout << "\n";
    
    Logger::log(LogLevel::INFO, "Example completed successfully!");
    
    return 0;
}
