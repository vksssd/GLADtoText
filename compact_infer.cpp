#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

struct CompactModel {
    int dim;
    std::unordered_map<std::string, int> word2id;
    std::vector<std::string> labels;
    std::vector<float> embeddings;
    std::vector<float> attention;
    std::vector<float> prototypes;
    
    bool load(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            std::cerr << "Cannot open: " << filename << "\n";
            return false;
        }
        
        // Read magic
        int magic;
        in.read((char*)&magic, sizeof(magic));
        if (magic != 0x47414C43) {
            std::cerr << "Not a compact model\n";
            return false;
        }
        
        in.read((char*)&dim, sizeof(dim));
        
        // Read dictionary
        int nwords;
        in.read((char*)&nwords, sizeof(nwords));
        for (int i = 0; i < nwords; i++) {
            int len;
            in.read((char*)&len, sizeof(len));
            std::string word(len, ' ');
            in.read(&word[0], len);
            word2id[word] = i;
        }
        
        // Read labels
        int nlabels;
        in.read((char*)&nlabels, sizeof(nlabels));
        for (int i = 0; i < nlabels; i++) {
            int len;
            in.read((char*)&len, sizeof(len));
            std::string label(len, ' ');
            in.read(&label[0], len);
            labels.push_back(label);
        }
        
        // Read embeddings
        embeddings.resize(nwords * dim);
        in.read((char*)embeddings.data(), nwords * dim * sizeof(float));
        
        // Read attention
        attention.resize(dim);
        in.read((char*)attention.data(), dim * sizeof(float));
        
        // Read prototypes
        prototypes.resize(nlabels * dim);
        in.read((char*)prototypes.data(), nlabels * dim * sizeof(float));
        
        in.close();
        return true;
    }
    
    std::vector<float> getEmbedding(int id) {
        std::vector<float> emb(dim);
        for (int d = 0; d < dim; d++) {
            emb[d] = embeddings[id * dim + d];
        }
        return emb;
    }
    
    std::vector<float> applyAttention(const std::vector<float>& emb) {
        std::vector<float> result(dim);
        for (int d = 0; d < dim; d++) {
            float a = 1.0f / (1.0f + std::exp(-attention[d] * emb[d]));
            result[d] = a * emb[d];
        }
        return result;
    }
    
    std::vector<float> aggregate(const std::vector<int>& ids) {
        std::vector<float> h(dim, 0.0f);
        for (int id : ids) {
            auto emb = getEmbedding(id);
            auto att_emb = applyAttention(emb);
            for (int d = 0; d < dim; d++) {
                h[d] += att_emb[d];
            }
        }
        return h;
    }
    
    float dot(const std::vector<float>& a, const std::vector<float>& b) {
        float sum = 0.0f;
        for (size_t i = 0; i < a.size(); i++) {
            sum += a[i] * b[i];
        }
        return sum;
    }
    
    std::vector<std::pair<std::string, float>> predict(const std::string& text, int k = 1) {
        // Tokenize
        std::istringstream iss(text);
        std::string word;
        std::vector<int> ids;
        
        while (iss >> word) {
            auto it = word2id.find(word);
            if (it != word2id.end()) {
                ids.push_back(it->second);
            }
        }
        
        if (ids.empty()) {
            return {};
        }
        
        // Get representation
        auto h = aggregate(ids);
        
        // Compute scores
        std::vector<std::pair<std::string, float>> results;
        for (size_t i = 0; i < labels.size(); i++) {
            std::vector<float> proto(dim);
            for (int d = 0; d < dim; d++) {
                proto[d] = prototypes[i * dim + d];
            }
            float score = dot(h, proto);
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
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <model.bin> [k]\n";
        std::cout << "Reads text from stdin, outputs predictions\n";
        return 1;
    }
    
    std::string modelFile = argv[1];
    int k = (argc > 2) ? std::stoi(argv[2]) : 1;
    
    CompactModel model;
    if (!model.load(modelFile)) {
        return 1;
    }
    
    std::cout << "Compact model loaded: " << model.word2id.size() 
              << " words, " << model.labels.size() << " labels, dim=" 
              << model.dim << "\n";
    
    std::string line;
    while (std::getline(std::cin, line)) {
        auto predictions = model.predict(line, k);
        for (const auto& pred : predictions) {
            std::cout << pred.first << " " << pred.second << "\n";
        }
    }
    
    return 0;
}
