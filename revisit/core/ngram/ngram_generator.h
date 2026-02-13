#pragma once

#include <string>
#include <string_view>
#include <vector>

class NGramGenerator {
public:
    NGramGenerator(int min_n, int max_n)
        : min_n_(min_n), max_n_(max_n) {}
    
    void generate(const std::string& word,
                  std::string& wrapped,
                  std::vector<std::string_view>& ngrams) const {
        // Wrap word with boundary markers
        wrapped.clear();
        wrapped.reserve(word.size() + 2);
        wrapped += '<';
        wrapped += word;
        wrapped += '>';
        
        ngrams.clear();
        
        // Generate all n-grams
        for (size_t i = 0; i < wrapped.size(); ++i) {
            for (int n = min_n_; n <= max_n_ && i + n <= wrapped.size(); ++n) {
                ngrams.emplace_back(wrapped.data() + i, n);
            }
        }
    }
    
    int min_n() const { return min_n_; }
    int max_n() const { return max_n_; }
    
private:
    int min_n_;
    int max_n_;
};
