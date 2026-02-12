#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "core/config.h"
#include "core/dictionary.h"
#include "subwords/char_ngram.h"
#include "model/embeddings.h"
#include "model/attention.h"
#include "model/backbone.h"
#include "heads/classify.h"

// Compact model format - only saves used embeddings

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string word;
    while (iss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

void trainCompactClassifier(const std::string& input_file,
                            const std::string& output_file,
                            int dim,
                            int epoch,
                            float lr) {
    
    std::cout << "Training compact classifier...\n";
    
    // Build dictionary and labels
    Dictionary dict;
    std::unordered_map<std::string, int> label2id;
    std::vector<std::string> id2label;
    std::unordered_map<std::string, int> wordCounts;
    
    std::ifstream file(input_file);
    std::string line;
    
    while (std::getline(file, line)) {
        auto tokens = tokenize(line);
        for (const auto& word : tokens) {
            if (word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__") {
                if (label2id.find(word) == label2id.end()) {
                    label2id[word] = id2label.size();
                    id2label.push_back(word);
                }
            } else {
                wordCounts[word]++;
            }
        }
    }
    
    // Only add words that appear in training data
    for (const auto& kv : wordCounts) {
        dict.addWord(kv.first);
    }
    
    // NO char n-grams, NO bucket - only word embeddings
    dict.finalize(0);  // bucket_size = 0
    
    std::cout << "Vocabulary: " << dict.id2word.size() << " words, "
              << id2label.size() << " labels\n";
    
    // Initialize model - only vocab_size embeddings (no buckets)
    int vocabSize = dict.id2word.size();
    Embeddings emb(vocabSize, dim);
    VectorAttention att(dim);
    Backbone backbone(&emb, &att, dim);
    Classifier clf(id2label.size(), dim, &backbone, lr);
    
    // Training loop
    for (int e = 0; e < epoch; e++) {
        std::ifstream file(input_file);
        std::string line;
        int processed = 0;
        float totalLoss = 0.0f;
        
        while (std::getline(file, line)) {
            auto tokens = tokenize(line);
            
            int label = -1;
            std::vector<int> textIds;
            
            for (const auto& word : tokens) {
                if (word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__") {
                    label = label2id[word];
                } else {
                    int wid = dict.getWordId(word);
                    if (wid >= 0) {
                        textIds.push_back(wid);
                    }
                }
            }
            
            if (label >= 0 && !textIds.empty()) {
                float loss = clf.trainExample(textIds, label);
                totalLoss += loss;
                processed++;
            }
        }
        
        if ((e + 1) % 10 == 0) {
            std::cout << "Epoch " << (e + 1) << "/" << epoch 
                     << " - Loss: " << (totalLoss / processed) << "\n";
        }
    }
    
    // Save compact model
    std::ofstream out(output_file, std::ios::binary);
    
    // Magic number for compact format
    int magic = 0x47414C43; // "GALC" (compact)
    out.write((char*)&magic, sizeof(magic));
    out.write((char*)&dim, sizeof(dim));
    
    // Save dictionary
    int nwords = dict.id2word.size();
    out.write((char*)&nwords, sizeof(nwords));
    for (const auto& word : dict.id2word) {
        int len = word.size();
        out.write((char*)&len, sizeof(len));
        out.write(word.c_str(), len);
    }
    
    // Save labels
    int nlabels = id2label.size();
    out.write((char*)&nlabels, sizeof(nlabels));
    for (const auto& label : id2label) {
        int len = label.size();
        out.write((char*)&len, sizeof(len));
        out.write(label.c_str(), len);
    }
    
    // Save only used embeddings (vocab_size, not vocab_size + bucket)
    for (int i = 0; i < vocabSize; i++) {
        for (int d = 0; d < dim; d++) {
            float val = emb.E.w[i * dim + d];
            out.write((char*)&val, sizeof(val));
        }
    }
    
    // Save attention (small)
    for (int d = 0; d < dim; d++) {
        float val = att.w.v[d];
        out.write((char*)&val, sizeof(val));
    }
    
    // Save classifier prototypes
    for (int i = 0; i < (int)id2label.size(); i++) {
        for (int d = 0; d < dim; d++) {
            float val = clf.prototypes.w[i * dim + d];
            out.write((char*)&val, sizeof(val));
        }
    }
    
    out.close();
    
    // Calculate size
    std::ifstream in(output_file, std::ios::binary | std::ios::ate);
    int size_bytes = in.tellg();
    int size_kb = size_bytes / 1024;
    
    std::cout << "\nModel saved: " << output_file << "\n";
    std::cout << "Size: " << size_kb << " KB\n";
    std::cout << "Breakdown:\n";
    std::cout << "  - Embeddings: " << (vocabSize * dim * 4 / 1024) << " KB\n";
    std::cout << "  - Prototypes: " << (id2label.size() * dim * 4 / 1024) << " KB\n";
    std::cout << "  - Attention: " << (dim * 4 / 1024) << " KB\n";
    std::cout << "  - Metadata: " << (size_kb - vocabSize * dim * 4 / 1024 - id2label.size() * dim * 4 / 1024) << " KB\n";
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <input> <output> [dim] [epoch] [lr]\n";
        std::cout << "Example: " << argv[0] << " intents.txt model.bin 20 50 0.2\n";
        return 1;
    }
    
    std::string input = argv[1];
    std::string output = argv[2];
    int dim = (argc > 3) ? std::stoi(argv[3]) : 20;
    int epoch = (argc > 4) ? std::stoi(argv[4]) : 50;
    float lr = (argc > 5) ? std::stof(argv[5]) : 0.2f;
    
    trainCompactClassifier(input, output, dim, epoch, lr);
    
    return 0;
}
