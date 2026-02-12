#pragma once
#include <unordered_map>
#include <vector>
#include <string>

struct Dictionary {
    // Word vocabulary
    std::unordered_map<std::string, int> word2id;
    std::vector<std::string> id2word;

    // Grammar units (frozen)
    std::unordered_map<std::string, int> grammar2id;
    int grammar_offset = 0;

    // Phonetic units
    std::unordered_map<std::string, int> phonetic2id;
    int phonetic_offset = 0;

    // Char n-grams (hashed)
    int bucket_size = 0;
    int bucket_offset = 0;

    // --- WORDS ---

    int addWord(const std::string& w) {
        auto it = word2id.find(w);
        if (it != word2id.end()) return it->second;

        int id = id2word.size();
        word2id[w] = id;
        id2word.push_back(w);
        return id;
    }

    int getWordId(const std::string& w) const {
        auto it = word2id.find(w);
        return (it != word2id.end()) ? it->second : -1;
    }

    // --- CHAR N-GRAMS (HASHED) ---

    int getCharNgramId(int hash) const {
        return bucket_offset + hash;
    }

    // --- GRAMMAR UNITS (FROZEN) ---

    void loadGrammarUnits(
        const std::unordered_map<std::string, int>& vocab
    ) {
        grammar_offset = id2word.size() + bucket_size;
        for (auto& kv : vocab) {
            grammar2id[kv.first] = grammar_offset + kv.second;
        }
    }

    int getGrammarId(const std::string& s) const {
        auto it = grammar2id.find(s);
        return (it != grammar2id.end()) ? it->second : -1;
    }

    // --- PHONETIC UNITS ---

    int addPhonetic(const std::string& p) {
        auto it = phonetic2id.find(p);
        if (it != phonetic2id.end()) return it->second;

        int id = phonetic_offset + phonetic2id.size();
        phonetic2id[p] = id;
        return id;
    }

    int getPhoneticId(const std::string& p) const {
        auto it = phonetic2id.find(p);
        return (it != phonetic2id.end()) ? it->second : -1;
    }

    // --- FINALIZE OFFSETS ---

    void finalize(int char_bucket_size) {
        bucket_size = char_bucket_size;
        bucket_offset = id2word.size();
        phonetic_offset = bucket_offset + bucket_size + grammar2id.size();
    }

    // --- TOTAL SIZE ---

    int totalSize() const {
        return phonetic_offset + phonetic2id.size();
    }
};
