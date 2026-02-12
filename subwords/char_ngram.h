#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../core/config.h"

/**
 * FastText-style character n-gram extractor with FNV-1a hashing
 * Extracts character n-grams from words for subword-level features
 */

class CharNgrams {
public:
    explicit CharNgrams(const Config& cfg)
        : minn(cfg.char_ngram_min),
          maxn(cfg.char_ngram_max),
          bucket(cfg.bucket_size) {
        if (minn < 1 || maxn < minn || bucket <= 0) {
            throw std::invalid_argument("Invalid char n-gram configuration");
        }
    }

    // Extract character n-grams from a word
    void extract(const std::string& word, std::vector<int>& ngrams) const {
        if (word.empty()) return;

        // Add word boundaries
        std::string w = "<" + word + ">";
        const int len = static_cast<int>(w.size());

        for (int i = 0; i < len; i++) {
            uint32_t h = 2166136261u; // FNV-1a base (offset basis)
            
            for (int j = i; j < len && j < i + maxn; j++) {
                // FNV-1a hash: XOR then multiply
                h ^= static_cast<uint8_t>(w[j]);
                h *= 16777619u; // FNV-1a prime

                int n = j - i + 1;
                if (n >= minn && n <= maxn) {
                    ngrams.push_back(static_cast<int>(h % bucket));
                }
            }
        }
    }
    
    // Get number of n-grams for a word (without extracting)
    int count(const std::string& word) const {
        if (word.empty()) return 0;
        
        std::string w = "<" + word + ">";
        const int len = static_cast<int>(w.size());
        int count = 0;
        
        for (int i = 0; i < len; i++) {
            for (int j = i; j < len && j < i + maxn; j++) {
                int n = j - i + 1;
                if (n >= minn && n <= maxn) {
                    count++;
                }
            }
        }
        
        return count;
    }

private:
    int minn;
    int maxn;
    int bucket;
};
