#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "core/config.h"
#include "core/dictionary.h"
#include "subwords/char_ngram.h"
#include "subwords/grammar_units.h"
#include "subwords/phonetic.h"
#include "model/embeddings.h"
#include "model/attention.h"
#include "model/backbone.h"
#include "model/sentence_encoder.h"
#include "heads/semantic.h"
#include "heads/classify.h"

void printUsage() {
    std::cout << "GLADtoTEXT - Custom FastText-inspired Text Embeddings\n\n";
    std::cout << "Usage:\n";
    std::cout << "  Train unsupervised (CBOW):\n";
    std::cout << "    ./gladtotext cbow -input data.txt -output model -dim 100 -epoch 5\n\n";
    std::cout << "  Train supervised (classification):\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -dim 100 -epoch 5\n\n";
    std::cout << "  Train supervised with pretrained embeddings:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -pretrained cbow_model.bin -epoch 5\n\n";
    std::cout << "  Train supervised with sentence encoding:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -dim 100 -epoch 5 -sentence\n\n";
    std::cout << "  Get word vector:\n";
    std::cout << "    ./gladtotext print-word-vector model.bin < words.txt\n\n";
    std::cout << "  Predict:\n";
    std::cout << "    ./gladtotext predict model.bin < test.txt\n\n";
}

struct Args {
    std::string command;
    std::string input;
    std::string output;
    std::string pretrained;  // NEW: pretrained model path
    int dim = 100;
    int epoch = 5;
    float lr = 0.05f;
    int negatives = 5;
    int window = 5;
    int minCount = 5;
    int minn = 3;
    int maxn = 6;
    int bucket = 2000000;
    bool useSentenceEncoder = false;  // NEW: enable sentence-level encoding
    
    bool parse(int argc, char** argv) {
        if (argc < 2) return false;
        command = argv[1];
        
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-input" && i + 1 < argc) input = argv[++i];
            else if (arg == "-output" && i + 1 < argc) output = argv[++i];
            else if (arg == "-pretrained" && i + 1 < argc) pretrained = argv[++i];  // NEW
            else if (arg == "-dim" && i + 1 < argc) dim = std::stoi(argv[++i]);
            else if (arg == "-epoch" && i + 1 < argc) epoch = std::stoi(argv[++i]);
            else if (arg == "-lr" && i + 1 < argc) lr = std::stof(argv[++i]);
            else if (arg == "-neg" && i + 1 < argc) negatives = std::stoi(argv[++i]);
            else if (arg == "-ws" && i + 1 < argc) window = std::stoi(argv[++i]);
            else if (arg == "-minCount" && i + 1 < argc) minCount = std::stoi(argv[++i]);
            else if (arg == "-minn" && i + 1 < argc) minn = std::stoi(argv[++i]);
            else if (arg == "-maxn" && i + 1 < argc) maxn = std::stoi(argv[++i]);
            else if (arg == "-bucket" && i + 1 < argc) bucket = std::stoi(argv[++i]);
            else if (arg == "-sentence") useSentenceEncoder = true;  // NEW
        }
        return true;
    }
};

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string word;
    while (iss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

bool loadPretrainedModel(const std::string& filename,
                         Dictionary& dict,
                         Embeddings*& emb,
                         VectorAttention*& att,
                         int& dim,
                         int& minn,
                         int& maxn,
                         int& bucket) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open pretrained model: " << filename << "\n";
        return false;
    }
    
    // Read metadata
    int magic;
    in.read((char*)&magic, sizeof(magic));
    
    if (magic != 0x47414C44 && magic != 0x47414C53) {
        std::cerr << "Invalid pretrained model format\n";
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
    
    // Skip labels if supervised model
    if (magic == 0x47414C53) {
        int nlabels;
        in.read((char*)&nlabels, sizeof(nlabels));
        for (int i = 0; i < nlabels; i++) {
            int len;
            in.read((char*)&len, sizeof(len));
            in.seekg(len, std::ios::cur); // Skip label
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
    
    in.close();
    
    std::cout << "Loaded pretrained model: " << dict.id2word.size() 
              << " words, dim=" << dim << "\n";
    
    return true;
}

void buildDictionary(const std::string& filename, Dictionary& dict, int minCount) {
    std::ifstream file(filename);
    std::string line;
    std::unordered_map<std::string, int> wordCounts;
    
    while (std::getline(file, line)) {
        auto tokens = tokenize(line);
        for (const auto& word : tokens) {
            if (word[0] != '_') { // Skip labels
                wordCounts[word]++;
            }
        }
    }
    
    for (const auto& kv : wordCounts) {
        if (kv.second >= minCount) {
            dict.addWord(kv.first);
        }
    }
    
    std::cout << "Dictionary: " << dict.id2word.size() << " words\n";
}

std::vector<int> getSubwordIds(const std::string& word, 
                                const Dictionary& dict,
                                CharNgrams& ngrams) {
    std::vector<int> ids;
    
    // Add word ID if exists
    int wid = dict.getWordId(word);
    if (wid >= 0) ids.push_back(wid);
    
    // Add char n-gram IDs
    std::vector<int> hashes;
    ngrams.extract(word, hashes);
    for (int h : hashes) {
        ids.push_back(dict.getCharNgramId(h));
    }
    
    return ids;
}

void trainCBOW(const Args& args) {
    std::cout << "Training CBOW model...\n";
    
    // Build dictionary
    Dictionary dict;
    buildDictionary(args.input, dict, args.minCount);
    dict.finalize(args.bucket);
    
    // Setup config
    Config cfg;
    cfg.dim = args.dim;
    cfg.char_ngram_min = args.minn;
    cfg.char_ngram_max = args.maxn;
    cfg.bucket_size = args.bucket;
    
    CharNgrams ngrams(cfg);
    
    // Initialize model
    int vocabSize = dict.totalSize();
    Embeddings emb(vocabSize, args.dim);
    VectorAttention att(args.dim);
    Backbone backbone(&emb, &att, args.dim);
    SemanticTrainer trainer(dict.id2word.size(), args.dim, &backbone, args.lr, args.negatives);
    NegativeSampler sampler(dict.id2word.size());
    
    // Training loop
    for (int epoch = 0; epoch < args.epoch; epoch++) {
        std::ifstream file(args.input);
        std::string line;
        int processed = 0;
        
        while (std::getline(file, line)) {
            auto tokens = tokenize(line);
            
            for (size_t i = 0; i < tokens.size(); i++) {
                // Get context window
                std::vector<int> contextIds;
                for (int j = std::max(0, (int)i - args.window); 
                     j < std::min((int)tokens.size(), (int)i + args.window + 1); j++) {
                    if (j != (int)i) {
                        auto ids = getSubwordIds(tokens[j], dict, ngrams);
                        contextIds.insert(contextIds.end(), ids.begin(), ids.end());
                    }
                }
                
                if (!contextIds.empty()) {
                    int targetId = dict.getWordId(tokens[i]);
                    if (targetId >= 0) {
                        trainer.cbowUpdate(contextIds, targetId, sampler);
                    }
                }
            }
            
            if (++processed % 10000 == 0) {
                std::cout << "\rEpoch " << (epoch + 1) << "/" << args.epoch 
                         << " - Processed " << processed << " lines" << std::flush;
            }
        }
        std::cout << "\n";
    }
    
    // Save model
    std::ofstream out(args.output + ".bin", std::ios::binary);
    
    // Save metadata
    int magic = 0x47414C44; // "GALD"
    out.write((char*)&magic, sizeof(magic));
    out.write((char*)&args.dim, sizeof(args.dim));
    out.write((char*)&args.minn, sizeof(args.minn));
    out.write((char*)&args.maxn, sizeof(args.maxn));
    out.write((char*)&args.bucket, sizeof(args.bucket));
    
    // Save dictionary
    int nwords = dict.id2word.size();
    out.write((char*)&nwords, sizeof(nwords));
    for (const auto& word : dict.id2word) {
        int len = word.size();
        out.write((char*)&len, sizeof(len));
        out.write(word.c_str(), len);
    }
    
    // Save embeddings
    int embSize = emb.E.w.size();
    out.write((char*)&embSize, sizeof(embSize));
    out.write((char*)emb.E.w.data(), embSize * sizeof(float));
    
    // Save attention
    int attSize = att.w.v.size();
    out.write((char*)&attSize, sizeof(attSize));
    out.write((char*)att.w.v.data(), attSize * sizeof(float));
    
    out.close();
    std::cout << "Model saved to " << args.output << ".bin\n";
}

void trainSupervised(const Args& args) {
    std::cout << "Training supervised model...\n";
    
    // Check if using pretrained embeddings
    bool usePretrained = !args.pretrained.empty();
    Embeddings* emb = nullptr;
    VectorAttention* att = nullptr;
    Dictionary dict;
    int dim = args.dim;
    int minn = args.minn;
    int maxn = args.maxn;
    int bucket = args.bucket;
    
    if (usePretrained) {
        std::cout << "Loading pretrained embeddings from " << args.pretrained << "...\n";
        if (!loadPretrainedModel(args.pretrained, dict, emb, att, dim, minn, maxn, bucket)) {
            return;
        }
        std::cout << "Pretrained embeddings loaded successfully!\n";
    } else {
        // Build dictionary from scratch
        std::ifstream file(args.input);
        std::string line;
        std::unordered_map<std::string, int> wordCounts;
        
        while (std::getline(file, line)) {
            auto tokens = tokenize(line);
            for (const auto& word : tokens) {
                if (!(word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__")) {
                    wordCounts[word]++;
                }
            }
        }
        
        for (const auto& kv : wordCounts) {
            if (kv.second >= args.minCount) {
                dict.addWord(kv.first);
            }
        }
        
        dict.finalize(bucket);
        
        // Initialize embeddings from scratch
        int vocabSize = dict.totalSize();
        emb = new Embeddings(vocabSize, dim);
        att = new VectorAttention(dim);
    }
    
    // Collect labels
    std::unordered_map<std::string, int> label2id;
    std::vector<std::string> id2label;
    
    std::ifstream file(args.input);
    std::string line;
    
    while (std::getline(file, line)) {
        auto tokens = tokenize(line);
        for (const auto& word : tokens) {
            if (word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__") {
                if (label2id.find(word) == label2id.end()) {
                    label2id[word] = id2label.size();
                    id2label.push_back(word);
                }
            }
        }
    }
    
    std::cout << "Dictionary: " << dict.id2word.size() << " words, " 
              << id2label.size() << " labels\n";
    
    // Setup config
    Config cfg;
    cfg.dim = dim;
    cfg.char_ngram_min = minn;
    cfg.char_ngram_max = maxn;
    cfg.bucket_size = bucket;
    
    CharNgrams ngrams(cfg);
    
    // Initialize model
    Backbone backbone(emb, att, dim);
    SentenceEncoder sentEnc(dim, args.useSentenceEncoder);
    Classifier clf(id2label.size(), dim, &backbone, args.lr);
    
    if (args.useSentenceEncoder) {
        std::cout << "Sentence-level encoding ENABLED\n";
    }
    
    // Training loop
    for (int epoch = 0; epoch < args.epoch; epoch++) {
        std::ifstream file(args.input);
        std::string line;
        int processed = 0;
        float totalLoss = 0.0f;
        
        while (std::getline(file, line)) {
            auto tokens = tokenize(line);
            
            // Extract label and text
            int label = -1;
            std::vector<int> textIds;
            std::vector<std::string> words;
            
            for (const auto& word : tokens) {
                if (word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__") {
                    label = label2id[word];
                } else {
                    words.push_back(word);
                    auto ids = getSubwordIds(word, dict, ngrams);
                    textIds.insert(textIds.end(), ids.begin(), ids.end());
                }
            }
            
            if (label >= 0 && !textIds.empty()) {
                float loss;
                
                if (args.useSentenceEncoder) {
                    // Sentence-level: encode each word, then combine with sentence encoder
                    std::vector<Vector> wordEmbeddings;
                    for (const auto& word : words) {
                        auto ids = getSubwordIds(word, dict, ngrams);
                        if (!ids.empty()) {
                            wordEmbeddings.push_back(backbone.forward(ids));
                        }
                    }
                    
                    // Encode sentence
                    Vector sentenceVec = sentEnc.encode(wordEmbeddings);
                    
                    // Classify sentence vector
                    loss = clf.trainExampleWithVector(sentenceVec, label);
                } else {
                    // Word-level: simple bag-of-words
                    loss = clf.trainExample(textIds, label);
                }
                
                totalLoss += loss;
                processed++;
            }
            
            if (processed % 1000 == 0 && processed > 0) {
                std::cout << "\rEpoch " << (epoch + 1) << "/" << args.epoch 
                         << " - Loss: " << (totalLoss / processed) << std::flush;
            }
        }
        std::cout << "\n";
    }
    
    // Save model
    std::ofstream out(args.output + ".bin", std::ios::binary);
    
    // Save metadata
    int magic = 0x47414C53; // "GALS" (supervised)
    out.write((char*)&magic, sizeof(magic));
    out.write((char*)&dim, sizeof(dim));
    out.write((char*)&minn, sizeof(minn));
    out.write((char*)&maxn, sizeof(maxn));
    out.write((char*)&bucket, sizeof(bucket));
    
    // Save sentence encoder flag
    int sentenceFlag = args.useSentenceEncoder ? 1 : 0;
    out.write((char*)&sentenceFlag, sizeof(sentenceFlag));
    
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
    
    // Save embeddings
    int embSize = emb->E.w.size();
    out.write((char*)&embSize, sizeof(embSize));
    out.write((char*)emb->E.w.data(), embSize * sizeof(float));
    
    // Save attention
    int attSize = att->w.v.size();
    out.write((char*)&attSize, sizeof(attSize));
    out.write((char*)att->w.v.data(), attSize * sizeof(float));
    
    // Save classifier prototypes
    int protoSize = clf.prototypes.w.size();
    out.write((char*)&protoSize, sizeof(protoSize));
    out.write((char*)clf.prototypes.w.data(), protoSize * sizeof(float));
    
    // Save sentence encoder if enabled
    if (args.useSentenceEncoder) {
        // Save attention matrices (query, key, value)
        int attDim = sentEnc.attention->dim;
        out.write((char*)&attDim, sizeof(attDim));
        
        // Save query weights
        for (int i = 0; i < attDim; i++) {
            for (int j = 0; j < attDim; j++) {
                float val = sentEnc.attention->query_w.get(i, j);
                out.write((char*)&val, sizeof(val));
            }
        }
        
        // Save key weights
        for (int i = 0; i < attDim; i++) {
            for (int j = 0; j < attDim; j++) {
                float val = sentEnc.attention->key_w.get(i, j);
                out.write((char*)&val, sizeof(val));
            }
        }
        
        // Save value weights
        for (int i = 0; i < attDim; i++) {
            for (int j = 0; j < attDim; j++) {
                float val = sentEnc.attention->value_w.get(i, j);
                out.write((char*)&val, sizeof(val));
            }
        }
    }
    
    out.close();
    std::cout << "Model saved to " << args.output << ".bin\n";
    
    // Cleanup
    if (!usePretrained) {
        delete emb;
        delete att;
    }
}

int main(int argc, char** argv) {
    Args args;
    
    if (!args.parse(argc, argv)) {
        printUsage();
        return 1;
    }
    
    if (args.command == "cbow") {
        trainCBOW(args);
    } else if (args.command == "supervised") {
        trainSupervised(args);
    } else {
        std::cout << "Unknown command: " << args.command << "\n";
        printUsage();
        return 1;
    }
    
    return 0;
}
