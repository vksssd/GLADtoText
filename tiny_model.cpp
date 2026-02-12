#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include "core/config.h"
#include "core/dictionary.h"
#include "subwords/char_ngram.h"
#include "subwords/grammar_units.h"
#include "subwords/phonetic.h"
#include "model/tiny_embeddings.h"
#include "model/attention.h"

// Quantize float to int8 (-128 to 127)
inline int8_t quantize(float val, float scale) {
    int scaled = static_cast<int>(val / scale);
    return static_cast<int8_t>(std::max(-128, std::min(127, scaled)));
}

inline float dequantize(int8_t val, float scale) {
    return static_cast<float>(val) * scale;
}

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string word;
    while (iss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

struct TinyClassifier {
    Dictionary dict;
    std::vector<std::string> labels;
    TinyEmbeddings* emb = nullptr;
    VectorAttention* att = nullptr;
    SparseMatrix* prototypes = nullptr;
    CharNgrams* ngrams = nullptr;
    GrammarUnitInducer* grammar = nullptr;
    
    int dim;
    int minn, maxn, bucket;
    bool use_char_ngrams;
    bool use_grammar;
    bool use_phonetic;
    
    ~TinyClassifier() {
        delete emb;
        delete att;
        delete prototypes;
        delete ngrams;
        delete grammar;
    }
    
    void train(const std::string& input_file,
               int dimension,
               int epochs,
               float lr,
               bool char_ngrams = true,
               bool grammar_units = true,
               bool phonetic = true,
               int min_ngram = 3,
               int max_ngram = 6,
               int bucket_size = 100000) {
        
        dim = dimension;
        minn = min_ngram;
        maxn = max_ngram;
        bucket = bucket_size;
        use_char_ngrams = char_ngrams;
        use_grammar = grammar_units;
        use_phonetic = phonetic;
        
        std::cout << "Training tiny model with ALL features...\n";
        std::cout << "  Char n-grams: " << (char_ngrams ? "ON" : "OFF") << "\n";
        std::cout << "  Grammar units: " << (grammar_units ? "ON" : "OFF") << "\n";
        std::cout << "  Phonetic: " << (phonetic ? "ON" : "OFF") << "\n";
        
        // Phase 1: Build vocabulary and collect labels
        std::unordered_map<std::string, int> wordCounts;
        std::unordered_map<std::string, int> label2id;
        
        std::ifstream file(input_file);
        std::string line;
        
        while (std::getline(file, line)) {
            auto tokens = tokenize(line);
            for (const auto& word : tokens) {
                if (word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__") {
                    if (label2id.find(word) == label2id.end()) {
                        label2id[word] = labels.size();
                        labels.push_back(word);
                    }
                } else {
                    wordCounts[word]++;
                    if (grammar_units) {
                        // Observe for grammar units
                        if (!grammar) {
                            grammar = new GrammarUnitInducer(3, 6, 2);
                        }
                        grammar->observe(word);
                    }
                }
            }
        }
        
        // Add words to dictionary
        for (const auto& kv : wordCounts) {
            if (kv.second >= 2) {  // minCount = 2
                dict.addWord(kv.first);
            }
        }
        
        // Finalize grammar units
        std::unordered_map<std::string, int> grammar_vocab;
        if (grammar_units && grammar) {
            grammar->finalize(grammar_vocab);
            dict.loadGrammarUnits(grammar_vocab);
        }
        
        dict.finalize(char_ngrams ? bucket_size : 0);
        
        std::cout << "Vocabulary: " << dict.id2word.size() << " words, "
                  << labels.size() << " labels";
        if (grammar_units) {
            std::cout << ", " << grammar_vocab.size() << " grammar units";
        }
        std::cout << "\n";
        
        // Phase 2: Initialize models (sparse)
        int vocab_size = dict.totalSize();
        emb = new TinyEmbeddings(vocab_size, dim);
        att = new VectorAttention(dim);
        prototypes = new SparseMatrix(labels.size(), dim);
        
        // Initialize prototypes
        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
        for (size_t i = 0; i < labels.size(); i++) {
            for (int d = 0; d < dim; d++) {
                prototypes->set(i, d, dist(gen));
            }
        }
        
        if (char_ngrams) {
            Config cfg;
            cfg.char_ngram_min = minn;
            cfg.char_ngram_max = maxn;
            cfg.bucket_size = bucket;
            ngrams = new CharNgrams(cfg);
        }
        
        // Phase 3: Training
        for (int epoch = 0; epoch < epochs; epoch++) {
            std::ifstream file(input_file);
            std::string line;
            float total_loss = 0.0f;
            int processed = 0;
            
            while (std::getline(file, line)) {
                auto tokens = tokenize(line);
                
                int label = -1;
                std::vector<int> text_ids;
                
                for (const auto& word : tokens) {
                    if (word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__") {
                        label = label2id[word];
                    } else {
                        // Get subword IDs
                        auto ids = getSubwordIds(word);
                        text_ids.insert(text_ids.end(), ids.begin(), ids.end());
                    }
                }
                
                if (label >= 0 && !text_ids.empty()) {
                    float loss = trainExample(text_ids, label, lr);
                    total_loss += loss;
                    processed++;
                }
            }
            
            // Prune every 10 epochs to save memory
            if ((epoch + 1) % 10 == 0) {
                emb->prune(1e-4f);
                prototypes->prune(1e-4f);
                
                if ((epoch + 1) % 20 == 0) {
                    size_t mem_kb = (emb->memoryUsage() + prototypes->memoryUsage()) / 1024;
                    std::cout << "Epoch " << (epoch + 1) << "/" << epochs
                             << " - Loss: " << (total_loss / processed)
                             << " - Memory: " << mem_kb << " KB\n";
                }
            }
        }
        
        // Final pruning
        emb->prune(1e-3f);
        prototypes->prune(1e-3f);
        
        std::cout << "\nTraining complete!\n";
    }
    
    std::vector<int> getSubwordIds(const std::string& word) {
        std::vector<int> ids;
        
        // 1. Word ID
        int wid = dict.getWordId(word);
        if (wid >= 0) {
            ids.push_back(wid);
            emb->initializeIfNeeded(wid);
        }
        
        // 2. Char n-grams
        if (use_char_ngrams && ngrams) {
            std::vector<int> hashes;
            ngrams->extract(word, hashes);
            for (int h : hashes) {
                int id = dict.getCharNgramId(h);
                ids.push_back(id);
                emb->initializeIfNeeded(id);
            }
        }
        
        // 3. Grammar units
        if (use_grammar) {
            for (size_t i = 0; i < word.size(); i++) {
                for (int len = 3; len <= 6 && i + len <= word.size(); len++) {
                    std::string unit = word.substr(i, len);
                    int gid = dict.getGrammarId(unit);
                    if (gid >= 0) {
                        ids.push_back(gid);
                        emb->initializeIfNeeded(gid);
                    }
                }
            }
        }
        
        // 4. Phonetic
        if (use_phonetic) {
            std::string phon = phoneticEncode(word);
            int pid = dict.getPhoneticId(phon);
            if (pid >= 0) {
                ids.push_back(pid);
                emb->initializeIfNeeded(pid);
            } else {
                pid = dict.addPhonetic(phon);
                ids.push_back(pid);
                emb->initializeIfNeeded(pid);
            }
        }
        
        return ids;
    }
    
    float trainExample(const std::vector<int>& ids, int label, float lr) {
        // Forward: aggregate embeddings
        Vector h(dim);
        h.zero();
        
        for (int id : ids) {
            Vector e = emb->lookup(id);
            Vector ae = att->apply(e);
            h.add(ae);
        }
        
        // Compute scores
        std::vector<float> scores(labels.size());
        for (size_t k = 0; k < labels.size(); k++) {
            scores[k] = prototypes->row(k).dot(h);
        }
        
        // Softmax loss
        float max_score = *std::max_element(scores.begin(), scores.end());
        float sum_exp = 0.0f;
        for (float s : scores) {
            sum_exp += std::exp(s - max_score);
        }
        float loss = -scores[label] + max_score + std::log(sum_exp);
        
        // Backward
        for (size_t k = 0; k < labels.size(); k++) {
            float prob = std::exp(scores[k] - max_score) / sum_exp;
            float grad_score = (k == label) ? prob - 1.0f : prob;
            
            // Update prototype
            Vector grad_p = h;
            grad_p.scale(-lr * grad_score);
            prototypes->addRow(grad_p, k, 1.0f);
            
            // Backprop to embeddings
            Vector grad_h = prototypes->row(k);
            grad_h.scale(-lr * grad_score);
            
            for (int id : ids) {
                Vector e = emb->lookup(id);
                Vector grad_e(dim);
                
                for (int d = 0; d < dim; d++) {
                    float wd = att->w.v[d];
                    float ed = e.v[d];
                    float a = 1.0f / (1.0f + std::exp(-wd * ed));
                    float da = a * (1.0f - a);
                    
                    // Update attention
                    att->w.v[d] += lr * grad_h.v[d] * ed * da * 0.1f;  // Smaller lr for attention
                    
                    // Gradient for embedding
                    grad_e.v[d] = grad_h.v[d] * (a + ed * da * wd);
                }
                
                emb->update(id, grad_e, lr);
            }
        }
        
        return loss;
    }
    
    void save(const std::string& filename) {
        std::ofstream out(filename, std::ios::binary);
        
        // Magic for tiny model
        int magic = 0x47414C54; // "GALT" (tiny)
        out.write((char*)&magic, sizeof(magic));
        out.write((char*)&dim, sizeof(dim));
        out.write((char*)&minn, sizeof(minn));
        out.write((char*)&maxn, sizeof(maxn));
        out.write((char*)&bucket, sizeof(bucket));
        
        // Flags
        out.write((char*)&use_char_ngrams, sizeof(use_char_ngrams));
        out.write((char*)&use_grammar, sizeof(use_grammar));
        out.write((char*)&use_phonetic, sizeof(use_phonetic));
        
        // Dictionary
        int nwords = dict.id2word.size();
        out.write((char*)&nwords, sizeof(nwords));
        for (const auto& word : dict.id2word) {
            int len = word.size();
            out.write((char*)&len, sizeof(len));
            out.write(word.c_str(), len);
        }
        
        // Labels
        int nlabels = labels.size();
        out.write((char*)&nlabels, sizeof(nlabels));
        for (const auto& label : labels) {
            int len = label.size();
            out.write((char*)&len, sizeof(len));
            out.write(label.c_str(), len);
        }
        
        // Save sparse embeddings with quantization
        float emb_scale = 0.01f;  // Quantization scale
        out.write((char*)&emb_scale, sizeof(emb_scale));
        
        int num_embeddings = 0;
        for (const auto& row : emb->E.data) {
            num_embeddings++;
        }
        out.write((char*)&num_embeddings, sizeof(num_embeddings));
        
        for (const auto& row : emb->E.data) {
            int row_id = row.first;
            out.write((char*)&row_id, sizeof(row_id));
            
            int num_values = row.second.size();
            out.write((char*)&num_values, sizeof(num_values));
            
            for (const auto& col : row.second) {
                int col_id = col.first;
                int8_t val = quantize(col.second, emb_scale);
                out.write((char*)&col_id, sizeof(col_id));
                out.write((char*)&val, sizeof(val));
            }
        }
        
        // Save attention
        for (int d = 0; d < dim; d++) {
            float val = att->w.v[d];
            out.write((char*)&val, sizeof(val));
        }
        
        // Save sparse prototypes with quantization
        float proto_scale = 0.01f;
        out.write((char*)&proto_scale, sizeof(proto_scale));
        
        for (size_t i = 0; i < labels.size(); i++) {
            int num_values = 0;
            for (int d = 0; d < dim; d++) {
                if (std::abs(prototypes->get(i, d)) > 1e-6f) {
                    num_values++;
                }
            }
            out.write((char*)&num_values, sizeof(num_values));
            
            for (int d = 0; d < dim; d++) {
                float val = prototypes->get(i, d);
                if (std::abs(val) > 1e-6f) {
                    int8_t qval = quantize(val, proto_scale);
                    out.write((char*)&d, sizeof(d));
                    out.write((char*)&qval, sizeof(qval));
                }
            }
        }
        
        out.close();
        
        // Report size
        std::ifstream in(filename, std::ios::binary | std::ios::ate);
        size_t size_bytes = in.tellg();
        size_t size_kb = size_bytes / 1024;
        
        std::cout << "\nModel saved: " << filename << "\n";
        std::cout << "Size: " << size_kb << " KB\n";
        std::cout << "Features:\n";
        std::cout << "  - Words: " << dict.id2word.size() << "\n";
        std::cout << "  - Char n-grams: " << (use_char_ngrams ? "YES" : "NO") << "\n";
        std::cout << "  - Grammar units: " << (use_grammar ? "YES" : "NO") << "\n";
        std::cout << "  - Phonetic: " << (use_phonetic ? "YES" : "NO") << "\n";
        std::cout << "  - Dimension: " << dim << "\n";
        std::cout << "  - Memory usage: " << (emb->memoryUsage() + prototypes->memoryUsage()) / 1024 << " KB\n";
    }
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <input> <output> [options]\n";
        std::cout << "Options:\n";
        std::cout << "  -dim <int>        Dimension (default: 30)\n";
        std::cout << "  -epoch <int>      Epochs (default: 50)\n";
        std::cout << "  -lr <float>       Learning rate (default: 0.1)\n";
        std::cout << "  -no-ngrams        Disable char n-grams\n";
        std::cout << "  -no-grammar       Disable grammar units\n";
        std::cout << "  -no-phonetic      Disable phonetic encoding\n";
        std::cout << "  -bucket <int>     Bucket size (default: 100000)\n";
        return 1;
    }
    
    std::string input = argv[1];
    std::string output = argv[2];
    
    int dim = 30;
    int epoch = 50;
    float lr = 0.1f;
    bool char_ngrams = true;
    bool grammar = true;
    bool phonetic = true;
    int bucket = 100000;
    
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-dim" && i + 1 < argc) dim = std::stoi(argv[++i]);
        else if (arg == "-epoch" && i + 1 < argc) epoch = std::stoi(argv[++i]);
        else if (arg == "-lr" && i + 1 < argc) lr = std::stof(argv[++i]);
        else if (arg == "-no-ngrams") char_ngrams = false;
        else if (arg == "-no-grammar") grammar = false;
        else if (arg == "-no-phonetic") phonetic = false;
        else if (arg == "-bucket" && i + 1 < argc) bucket = std::stoi(argv[++i]);
    }
    
    TinyClassifier clf;
    clf.train(input, dim, epoch, lr, char_ngrams, grammar, phonetic, 3, 6, bucket);
    clf.save(output);
    
    return 0;
}
