#pragma once
#include <string>
#include <cctype>

/**
 * Enhanced Phonetic Encoder
 * - Adds standard-compliant first-character preservation.
 * - Implements safety casts for char handling.
 */
 
inline std::string phoneticEncode(const std::string& w) {
    if (w.empty()) return "";

    std::string out;
    // Standard Soundex usually keeps the first letter as a reference point
    out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(w[0]))));

    // Mapping for the very first char to prevent immediate duplicates
    auto get_code = [](char c) -> char {
        switch (std::tolower(static_cast<unsigned char>(c))) {
            case 'b': case 'f': case 'p': case 'v': return '1';
            case 'c': case 'g': case 'j': case 'k':
            case 'q': case 's': case 'x': case 'z': return '2';
            case 'd': case 't':                     return '3';
            case 'l':                               return '4';
            case 'm': case 'n':                     return '5';
            case 'r':                               return '6';
            default:                                return '0';
        }
    };

    char prev = get_code(w[0]);

    for (size_t i = 1; i < w.size(); ++i) {
        char x = get_code(w[i]);

        // Rules: Ignore vowels/h/w/y ('0') and don't repeat the same digit
        if (x != '0' && x != prev) {
            out.push_back(x);
        }
        prev = x;
    }

    // Optional: Standard Soundex truncates or pads to 4 characters (e.g., A123)
    // while (out.size() < 4) out.push_back('0');
    // if (out.size() > 4) out = out.substr(0, 4);

    return out;
}