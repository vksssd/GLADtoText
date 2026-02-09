#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../core/config.h"

/*
 fastText-style character n-gram extractor
*/

class CharNgrams {
public:
    explicit CharNgrams(const Config& cfg)
        : minn(cfg.char_ngram_min),
          maxn(cfg.char_ngram_max),
          bucket(cfg.bucket_size) {}

    void extract(const std::string& word,
                 std::vector<int>& ngrams) const {

        std::string w = "<" + word + ">";
        const int len = w.size();

        for (int i = 0; i < len; i++) {
            uint32_t h = 2166136261; // FNV-1a base
            for (int j = i; j < len && j < i + maxn; j++) {
                h ^= static_cast<uint8_t>(w[j]);
                h *= 16777619;

                int n = j - i + 1;
                if (n >= minn) {
                    ngrams.push_back(h % bucket);
                }
            }
        }
    }

private:
    int minn;
    int maxn;
    int bucket;
};
