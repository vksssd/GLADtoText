#pragma once

#include <string>
#include <cctype>

class PhoneticEncoder {
public:
    // Simplified Soundex-like encoding
    std::string encode(const std::string& word) const {
        if (word.empty()) return "";
        
        std::string result;
        result.reserve(8);
        
        // Keep first letter
        char first = std::toupper(word[0]);
        result += first;
        
        char prev_code = get_code(first);
        
        // Encode remaining letters
        for (size_t i = 1; i < word.size() && result.size() < 8; ++i) {
            char c = std::toupper(word[i]);
            char code = get_code(c);
            
            // Skip vowels and duplicates
            if (code != '0' && code != prev_code) {
                result += code;
                prev_code = code;
            }
        }
        
        return result;
    }
    
private:
    char get_code(char c) const {
        switch (c) {
            case 'B': case 'F': case 'P': case 'V':
                return '1';
            case 'C': case 'G': case 'J': case 'K': case 'Q': case 'S': case 'X': case 'Z':
                return '2';
            case 'D': case 'T':
                return '3';
            case 'L':
                return '4';
            case 'M': case 'N':
                return '5';
            case 'R':
                return '6';
            default:
                return '0'; // Vowels and others
        }
    }
};
