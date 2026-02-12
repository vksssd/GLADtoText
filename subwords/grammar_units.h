#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

struct GrammarNode {
    // Using pointers for simplicity, though unique_ptr is safer for ownership
    std::unordered_map<char, GrammarNode*> next;
    int freq = 0;
    int id = -1;

    // Simple recursive destructor to prevent leaks
    ~GrammarNode() {
        for (auto& pair : next) delete pair.second;
    }
};

class GrammarTrie {
public:
    GrammarTrie() : root(new GrammarNode()) {}
    ~GrammarTrie() { delete root; }

    void addSubstring(const std::string& s) {
        GrammarNode* curr = root;
        for (char c : s) {
            if (!curr->next.count(c)) {
                curr->next[c] = new GrammarNode();
            }
            curr = curr->next[c];
        }
        curr->freq++;
    }

    void collect(int min_freq, int& next_id, std::unordered_map<std::string, int>& vocab) {
        std::string path = ""; 
        dfs(root, path, min_freq, next_id, vocab);
    }

private:
    GrammarNode* root;

    void dfs(GrammarNode* node, std::string& path, int min_freq, int& next_id, std::unordered_map<std::string, int>& vocab) {
        // If the current path represents a frequent substring, add to vocab
        if (node->freq >= min_freq && node->id == -1 && !path.empty()) {
            node->id = next_id++;
            vocab[path] = node->id;
        }

        for (auto& kv : node->next) {
            path.push_back(kv.first);
            dfs(kv.second, path, min_freq, next_id, vocab);
            path.pop_back();
        }
    }
};

class GrammarUnitInducer {
public:
    GrammarUnitInducer(int min_len, int max_len, int min_freq)
        : minLen(min_len), maxLen(max_len), minFreq(min_freq) {}

    void observe(const std::string& word) {
        const int n = word.size();
        for (int i = 0; i < n; i++) {
            // FIXED: loop now goes from minLen to maxLen
            for (int l = minLen; l <= maxLen && i + l <= n; l++) {
                trie.addSubstring(word.substr(i, l));
            }
        }
    }

    void finalize(std::unordered_map<std::string, int>& vocab) {
        int next_id = 0;
        trie.collect(minFreq, next_id, vocab);
    }

private:
    int minLen, maxLen, minFreq;
    GrammarTrie trie;
};