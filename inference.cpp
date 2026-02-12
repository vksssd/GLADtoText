#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "core/config.h"
#include "core/dictionary.h"
#include "subwords/char_ngram.h"
#include "model/embeddings.h"
#include "model/attention.h"
#include "model/backbone.h"
#include "heads/classify.h"

struct Model {
    int dim;
    int minn, maxn, bucket;
    Dictionary dict;
    Embeddings* emb = nullptr;
    VectorAttention* att = nullptr;
    Backbone* backbone = nullptr;
    Classifier* clf = nullptr;
    std::vector<std::string> labels;
    bool supervised = false;
    
    ~Model() {
        delete clf;
        delete backbone;
        delete att;
        delete emb;
    }
    
    bool load(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            std::cerr << "Cannot open model file: " << filename << "\n";
            return false;
        }
        
        // Read metadata
        int magic;
        in.read((char*)&magic, sizeof(magic));
        
        if (magic == 0x47414C53) {
            supervised = true;
        } else if (magic != 0x47414C44) {
            std::cerr << "Invalid model file format\n";
            return false;
        }
        
        in.read((char*)&dim, sizeof(dim));
        in.read((char*)&minn, sizeof(minn));
        in.read((char*)&maxn, sizeof(maxn));
        in.read((char*)&bucket, sizeof(bucket));
        
        // Read dictionary
        int nwords;
        in.read((char*)&nwords, sizeof(nwords));
        for (int i = 0; i < nwords; i++) {
            int len;
            in.read((char*)&len, sizeof(len));
            std::string word(len, ' ');
            in.read(&word[0], len);
            dict.addWord(word);
        }
        
        // Read labels if supervised
        if (supervised) {
            int nlabels;
            in.read((char*)&nlabels, sizeof(nlabels));
            for (int i = 0; i < nlabels; i++) {
                int len;
                in.read((char*)&len, sizeof(len));
                std::string label(len, ' ');
                in.read(&label[0], len);
                labels.push_back(label);
            }
        }
        
        dict.finalize(bucket);
        
        // Read embeddings
        int vocabSize = dict.totalSize();
        emb = new Embeddings(vocabSize, dim);
        int embSize;
        in.read((char*)&embSize, sizeof(embSize));
        in.read((char*)emb->E.w.data(), embSize * sizeof(float));
        
        // Read attention
        att = new VectorAttention(dim);
        int attSize;
        in.read((char*)&attSize, sizeof(attSize));
        in.read((char*)att->w.v.data(), attSize * sizeof(float));
        
        backbone = new Backbone(emb, att, dim);
        
        // Read classifier if supervised
        if (supervised) {
            clf = new Classifier(labels.size(), dim, backbone);
            int protoSize;
            in.read((char*)&protoSize, sizeof(protoSize));
            in.read((char*)clf->prototypes.w.data(), protoSize * sizeof(float));
        }
        
        in.close();
        return true;
    }
    
    std::vector<int> getSubwordIds(const std::string& word) {
        Config cfg;
        cfg.char_ngram_min = minn;
        cfg.char_ngram_max = maxn;
        cfg.bucket_size = bucket;
        CharNgrams ngrams(cfg);
        
        std::vector<int> ids;
        
        int wid = dict.getWordId(word);
        if (wid >= 0) ids.push_back(wid);
        
        std::vector<int> hashes;
        ngrams.extract(word, hashes);
        for (int h : hashes) {
            ids.push_back(dict.getCharNgramId(h));
        }
        
        return ids;
    }
    
    Vector getWordVector(const std::string& word) {
        auto ids = getSubwordIds(word);
        return backbone->forward(ids);
    }
    
    std::vector<std::pair<std::string, float>> predict(const std::string& text, int k = 1) {
        if (!supervised) {
            std::cerr << "Model is not supervised\n";
            return {};
        }
        
        // Tokenize
        std::istringstream iss(text);
        std::string word;
        std::vector<int> textIds;
        
        while (iss >> word) {
            auto ids = getSubwordIds(word);
            textIds.insert(textIds.end(), ids.begin(), ids.end());
        }
        
        if (textIds.empty()) return {};
        
        // Get representation
        Vector h = backbone->forward(textIds);
        
        // Compute scores
        std::vector<std::pair<std::string, float>> results;
        for (size_t i = 0; i < labels.size(); i++) {
            float score = clf->prototypes.row(i).dot(h);
            results.push_back({labels[i], score});
        }
        
        // Sort by score
        std::sort(results.begin(), results.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (k < (int)results.size()) {
            results.resize(k);
        }
        
        return results;
    }
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage:\n";
        std::cout << "  Print word vectors: " << argv[0] << " print-word-vector model.bin\n";
        std::cout << "  Predict: " << argv[0] << " predict model.bin [k]\n";
        return 1;
    }
    
    std::string command = argv[1];
    std::string modelFile = argv[2];
    
    Model model;
    if (!model.load(modelFile)) {
        return 1;
    }
    
    std::cout << "Model loaded: " << model.dict.id2word.size() << " words, "
              << "dim=" << model.dim << "\n";
    
    if (command == "print-word-vector") {
        std::string word;
        while (std::cin >> word) {
            Vector v = model.getWordVector(word);
            std::cout << word;
            for (float x : v.v) {
                std::cout << " " << x;
            }
            std::cout << "\n";
        }
    } else if (command == "predict") {
        int k = (argc > 3) ? std::stoi(argv[3]) : 1;
        std::string line;
        while (std::getline(std::cin, line)) {
            auto predictions = model.predict(line, k);
            for (const auto& pred : predictions) {
                std::cout << pred.first << " " << pred.second << "\n";
            }
        }
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;
    }
    
    return 0;
}
