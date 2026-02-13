#pragma once

#include "itokenizer.h"
#include <cctype>

class EnglishTokenizer : public ITokenizer {
public:
    void tokenize(const std::string& text,
                  std::vector<std::string>& tokens) const override {
        tokens.clear();
        
        std::string current;
        current.reserve(32);
        
        for (char c : text) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                current += std::tolower(static_cast<unsigned char>(c));
            } else if (!current.empty()) {
                tokens.push_back(std::move(current));
                current.clear();
            }
        }
        
        if (!current.empty()) {
            tokens.push_back(std::move(current));
        }
    }
    
    // Convenience method that returns tokens
    std::vector<std::string> tokenize(const std::string& text) const {
        std::vector<std::string> tokens;
        tokenize(text, tokens);
        return tokens;
    }
};
