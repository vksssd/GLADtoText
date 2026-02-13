#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "core/config.h"
#include "core/model_config.h"
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
    std::cout << "  Train with batch processing:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -batch 64\n\n";
    std::cout << "  Train with attention downgrade (faster):\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -attention simple\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -attention none\n\n";
    std::cout << "  Train with config file:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -config model.config\n\n";
    std::cout << "  Train with validation:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -valid valid.txt -output model\n\n";
    std::cout << "  Train with early stopping:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -valid valid.txt -output model -early-stop -patience 3\n\n";
    std::cout << "  Train supervised with pretrained embeddings:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -pretrained cbow_model.bin -epoch 5\n\n";
    std::cout << "  Train supervised with sentence encoding:\n";
    std::cout << "    ./gladtotext supervised -input train.txt -output model -dim 100 -epoch 5 -sentence\n\n";
}

struct Args {
    std::string command;
    std::string input;
    std::string output;
    std::string pretrained;
    std::string configFile;  // NEW: config file path
    int dim = 100;
    int epoch = 5;
    float lr = 0.05f;
    int negatives = 5;
    int window = 5;
    int minCount = 5;
    int minn = 3;
    int maxn = 6;
    int bucket = 2000000;
    bool useSentenceEncoder = false;
    bool useGrammarUnits = true;
    bool usePhonetic = true;
    std::string validFile;  // NEW: validation file
    bool earlyStop = false;  // NEW: enable early stopping
    int patience = 3;        // NEW: early stopping patience
    int batchSize = 1;      // NEW: batch size for training (default 1 for compatibility)
    std::string attentionMode = "full";  // NEW: attention mode (full, simple, none)
    
    bool parse(int argc, char** argv) {
        if (argc < 2) return false;
        command = argv[1];
        
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-input" && i + 1 < argc) input = argv[++i];
            else if (arg == "-output" && i + 1 < argc) output = argv[++i];
            else if (arg == "-pretrained" && i + 1 < argc) pretrained = argv[++i];
            else if (arg == "-config" && i + 1 < argc) configFile = argv[++i];
            else if (arg == "-valid" && i + 1 < argc) validFile = argv[++i];  // NEW
            else if (arg == "-dim" && i + 1 < argc) dim = std::stoi(argv[++i]);
            else if (arg == "-epoch" && i + 1 < argc) epoch = std::stoi(argv[++i]);
            else if (arg == "-lr" && i + 1 < argc) lr = std::stof(argv[++i]);
            else if (arg == "-neg" && i + 1 < argc) negatives = std::stoi(argv[++i]);
            else if (arg == "-ws" && i + 1 < argc) window = std::stoi(argv[++i]);
            else if (arg == "-minCount" && i + 1 < argc) minCount = std::stoi(argv[++i]);
            else if (arg == "-minn" && i + 1 < argc) minn = std::stoi(argv[++i]);
            else if (arg == "-maxn" && i + 1 < argc) maxn = std::stoi(argv[++i]);
            else if (arg == "-bucket" && i + 1 < argc) bucket = std::stoi(argv[++i]);
            else if (arg == "-sentence") useSentenceEncoder = true;
            else if (arg == "-no-grammar") useGrammarUnits = false;
            else if (arg == "-no-phonetic") usePhonetic = false;
            else if (arg == "-early-stop") earlyStop = true;  // NEW
            else if (arg == "-patience" && i + 1 < argc) patience = std::stoi(argv[++i]);  // NEW
            else if (arg == "-batch" && i + 1 < argc) batchSize = std::stoi(argv[++i]);  // NEW
            else if (arg == "-attention" && i + 1 < argc) attentionMode = argv[++i];  // NEW
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

// Parse attention mode from string
AttentionMode parseAttentionMode(const std::string& mode) {
    if (mode == "full") return AttentionMode::FULL;
    if (mode == "simple") return AttentionMode::SIMPLE;
    if (mode == "none") return AttentionMode::NONE;
    
    std::cerr << "Warning: Unknown attention mode '" << mode 
              << "', using 'full' (options: full, simple, none)\n";
    return AttentionMode::FULL;
}

// Save ModelConfig to binary stream
void saveModelConfig(std::ofstream& out, const ModelConfig& config) {
    // Save layer activation flags
    out.write((char*)&config.use_word_embeddings, sizeof(bool));
    out.write((char*)&config.use_char_ngrams, sizeof(bool));
    out.write((char*)&config.use_grammar_units, sizeof(bool));
    out.write((char*)&config.use_phonetic, sizeof(bool));
    out.write((char*)&config.use_vector_attention, sizeof(bool));
    out.write((char*)&config.use_sentence_encoder, sizeof(bool));
    
    // Save model parameters
    out.write((char*)&config.dim, sizeof(int));
    out.write((char*)&config.bucket_size, sizeof(int));
    out.write((char*)&config.minn, sizeof(int));
    out.write((char*)&config.maxn, sizeof(int));
}

// Load ModelConfig from binary stream
void loadModelConfig(std::ifstream& in, ModelConfig& config) {
    // Load layer activation flags
    in.read((char*)&config.use_word_embeddings, sizeof(bool));
    in.read((char*)&config.use_char_ngrams, sizeof(bool));
    in.read((char*)&config.use_grammar_units, sizeof(bool));
    in.read((char*)&config.use_phonetic, sizeof(bool));
    in.read((char*)&config.use_vector_attention, sizeof(bool));
    in.read((char*)&config.use_sentence_encoder, sizeof(bool));
    
    // Load model parameters
    in.read((char*)&config.dim, sizeof(int));
    in.read((char*)&config.bucket_size, sizeof(int));
    in.read((char*)&config.minn, sizeof(int));
    in.read((char*)&config.maxn, sizeof(int));
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
    
    // Check version
    int version = 1;  // Default to version 1 for old models
    int firstInt;
    in.read((char*)&firstInt, sizeof(firstInt));
    
    // If firstInt is a reasonable version number (1-10), it's version 2+
    if (firstInt >= 1 && firstInt <= 10) {
        version = firstInt;
        
        // Read ModelConfig if version 2+
        if (version >= 2) {
            ModelConfig config;
            loadModelConfig(in, config);
            // Config is loaded but we'll still read legacy params for compatibility
        }
        
        // Read legacy parameters
        in.read((char*)&dim, sizeof(dim));
        in.read((char*)&minn, sizeof(minn));
        in.read((char*)&maxn, sizeof(maxn));
        in.read((char*)&bucket, sizeof(bucket));
    } else {
        // Version 1: firstInt is actually dim
        dim = firstInt;
        in.read((char*)&minn, sizeof(minn));
        in.read((char*)&maxn, sizeof(maxn));
        in.read((char*)&bucket, sizeof(bucket));
    }
    
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

void learnGrammarUnits(const std::string& filename, Dictionary& dict, int minCount) {
    std::cout << "Learning grammar units...\n";
    
    // Create grammar unit inducer
    GrammarUnitInducer inducer(2, 5, minCount);  // min_len=2, max_len=5, min_freq=minCount
    
    // Observe all words
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        auto tokens = tokenize(line);
        for (const auto& word : tokens) {
            if (word[0] != '_' && word.size() >= 2) {  // Skip labels and single chars
                inducer.observe(word);
            }
        }
    }
    
    // Extract frequent grammar units
    std::unordered_map<std::string, int> grammarVocab;
    inducer.finalize(grammarVocab);
    
    // Load into dictionary
    dict.loadGrammarUnits(grammarVocab);
    
    std::cout << "Learned " << grammarVocab.size() << " grammar units\n";
}

void learnPhoneticUnits(const std::string& filename, Dictionary& dict) {
    std::cout << "Learning phonetic encodings...\n";
    
    // Extract phonetic encodings for all words
    std::ifstream file(filename);
    std::string line;
    std::unordered_map<std::string, int> phoneticCounts;
    
    while (std::getline(file, line)) {
        auto tokens = tokenize(line);
        for (const auto& word : tokens) {
            if (word[0] != '_') {  // Skip labels
                std::string phonetic = phoneticEncode(word);
                phoneticCounts[phonetic]++;
            }
        }
    }
    
    // Add phonetic encodings to dictionary
    for (const auto& kv : phoneticCounts) {
        dict.addPhonetic(kv.first);
    }
    
    std::cout << "Learned " << phoneticCounts.size() << " phonetic encodings\n";
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
    
    // Add grammar unit IDs (if available in dictionary)
    for (const auto& kv : dict.grammar2id) {
        if (word.find(kv.first) != std::string::npos) {
            ids.push_back(kv.second);
        }
    }
    
    // Add phonetic ID (if available in dictionary)
    std::string phonetic = phoneticEncode(word);
    int pid = dict.getPhoneticId(phonetic);
    if (pid >= 0) ids.push_back(pid);
    
    return ids;
}

// Validation evaluation function
struct ValidationMetrics {
    float accuracy = 0.0f;
    float precision = 0.0f;
    float recall = 0.0f;
    float f1 = 0.0f;
    int correct = 0;
    int total = 0;
};

ValidationMetrics evaluateValidation(
    const std::string& validFile,
    const Dictionary& dict,
    CharNgrams& ngrams,
    Backbone& backbone,
    SentenceEncoder& sentEnc,
    Classifier& clf,
    const std::unordered_map<std::string, int>& label2id,
    bool useSentenceEncoder
) {
    ValidationMetrics metrics;
    
    std::ifstream file(validFile);
    if (!file) {
        std::cerr << "Warning: Cannot open validation file: " << validFile << "\n";
        return metrics;
    }
    
    std::string line;
    std::vector<int> perClassCorrect(label2id.size(), 0);
    std::vector<int> perClassTotal(label2id.size(), 0);
    std::vector<int> perClassPredicted(label2id.size(), 0);
    
    while (std::getline(file, line)) {
        auto tokens = tokenize(line);
        
        // Extract label and text
        int trueLabel = -1;
        std::vector<int> textIds;
        std::vector<std::string> words;
        
        for (const auto& word : tokens) {
            if (word[0] == '_' && word.size() > 2 && word.substr(0, 2) == "__") {
                auto it = label2id.find(word);
                if (it != label2id.end()) {
                    trueLabel = it->second;
                }
            } else {
                words.push_back(word);
                auto ids = getSubwordIds(word, dict, ngrams);
                textIds.insert(textIds.end(), ids.begin(), ids.end());
            }
        }
        
        if (trueLabel < 0 || textIds.empty()) continue;
        
        // Get representation
        Vector h;
        if (useSentenceEncoder) {
            std::vector<Vector> wordEmbeddings;
            for (const auto& word : words) {
                auto ids = getSubwordIds(word, dict, ngrams);
                if (!ids.empty()) {
                    wordEmbeddings.push_back(backbone.forward(ids));
                }
            }
            h = sentEnc.encode(wordEmbeddings);
        } else {
            h = backbone.forward(textIds);
        }
        
        // Predict
        auto predictions = clf.predictTopK(h, 1);
        int predictedLabel = predictions.empty() ? -1 : predictions[0].first;
        
        // Update metrics
        metrics.total++;
        perClassTotal[trueLabel]++;
        
        if (predictedLabel >= 0) {
            perClassPredicted[predictedLabel]++;
            
            if (predictedLabel == trueLabel) {
                metrics.correct++;
                perClassCorrect[trueLabel]++;
            }
        }
    }
    
    // Calculate metrics
    if (metrics.total > 0) {
        metrics.accuracy = (float)metrics.correct / metrics.total;
        
        // Calculate macro-averaged precision and recall
        float totalPrecision = 0.0f;
        float totalRecall = 0.0f;
        int numClasses = 0;
        
        for (size_t i = 0; i < label2id.size(); i++) {
            if (perClassTotal[i] > 0) {
                float recall = (float)perClassCorrect[i] / perClassTotal[i];
                totalRecall += recall;
                numClasses++;
            }
            if (perClassPredicted[i] > 0) {
                float precision = (float)perClassCorrect[i] / perClassPredicted[i];
                totalPrecision += precision;
            }
        }
        
        if (numClasses > 0) {
            metrics.precision = totalPrecision / numClasses;
            metrics.recall = totalRecall / numClasses;
            
            if (metrics.precision + metrics.recall > 0) {
                metrics.f1 = 2 * metrics.precision * metrics.recall / 
                            (metrics.precision + metrics.recall);
            }
        }
    }
    
    return metrics;
}

void trainCBOW(const Args& args) {
    std::cout << "Training CBOW model...\n";
    
    // Build dictionary
    Dictionary dict;
    buildDictionary(args.input, dict, args.minCount);
    
    // Learn grammar units and phonetic encodings
    if (args.useGrammarUnits) {
        learnGrammarUnits(args.input, dict, args.minCount);
    }
    if (args.usePhonetic) {
        learnPhoneticUnits(args.input, dict);
    }
    
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
    
    // Save metadata with version
    int magic = 0x47414C44; // "GALD"
    int version = 2;  // Version 2: includes ModelConfig
    out.write((char*)&magic, sizeof(magic));
    out.write((char*)&version, sizeof(version));
    
    // Save ModelConfig
    ModelConfig modelConfig;
    modelConfig.dim = args.dim;
    modelConfig.bucket_size = args.bucket;
    modelConfig.minn = args.minn;
    modelConfig.maxn = args.maxn;
    modelConfig.use_char_ngrams = true;
    modelConfig.use_grammar_units = args.useGrammarUnits;
    modelConfig.use_phonetic = args.usePhonetic;
    modelConfig.use_sentence_encoder = false;
    saveModelConfig(out, modelConfig);
    
    // Save legacy parameters for backward compatibility
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
    
    // Save grammar units
    int ngrammar = dict.grammar2id.size();
    out.write((char*)&ngrammar, sizeof(ngrammar));
    for (const auto& kv : dict.grammar2id) {
        int len = kv.first.size();
        out.write((char*)&len, sizeof(len));
        out.write(kv.first.c_str(), len);
        out.write((char*)&kv.second, sizeof(kv.second));
    }
    
    // Save phonetic units
    int nphonetic = dict.phonetic2id.size();
    out.write((char*)&nphonetic, sizeof(nphonetic));
    for (const auto& kv : dict.phonetic2id) {
        int len = kv.first.size();
        out.write((char*)&len, sizeof(len));
        out.write(kv.first.c_str(), len);
        out.write((char*)&kv.second, sizeof(kv.second));
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
        
        // Learn grammar units and phonetic encodings
        if (args.useGrammarUnits) {
            learnGrammarUnits(args.input, dict, args.minCount);
        }
        if (args.usePhonetic) {
            learnPhoneticUnits(args.input, dict);
        }
        
        dict.finalize(bucket);
        
        // Initialize embeddings from scratch
        int vocabSize = dict.totalSize();
        emb = new Embeddings(vocabSize, dim);
        
        // Parse and apply attention mode
        AttentionMode attMode = parseAttentionMode(args.attentionMode);
        att = new VectorAttention(dim, attMode);
        
        // Report attention mode
        std::cout << "Attention mode: " << args.attentionMode;
        if (attMode == AttentionMode::SIMPLE) {
            std::cout << " (faster, linear attention)";
        } else if (attMode == AttentionMode::NONE) {
            std::cout << " (fastest, no attention)";
        }
        std::cout << "\n";
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
    
    // Validation setup
    bool useValidation = !args.validFile.empty();
    float bestAccuracy = 0.0f;
    int patienceCounter = 0;
    std::string bestModelPath = args.output + "_best.bin";
    
    if (useValidation) {
        std::cout << "Validation enabled with file: " << args.validFile << "\n";
        if (args.earlyStop) {
            std::cout << "Early stopping enabled with patience: " << args.patience << "\n";
        }
    }
    
    // Training loop
    std::cout << "Batch size: " << args.batchSize << "\n";
    
    for (int epoch = 0; epoch < args.epoch; epoch++) {
        std::ifstream file(args.input);
        std::string line;
        int processed = 0;
        float totalLoss = 0.0f;
        
        // Batch accumulators
        std::vector<std::vector<int>> batchIds;
        std::vector<int> batchLabels;
        std::vector<std::vector<Vector>> batchWordEmbeddings;
        std::vector<std::vector<std::vector<int>>> batchWordIds;  // For sentence encoder backprop
        
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
                // Add to batch
                batchIds.push_back(textIds);
                batchLabels.push_back(label);
                
                if (args.useSentenceEncoder) {
                    // Prepare word embeddings and IDs for sentence encoding
                    std::vector<Vector> wordEmbeddings;
                    std::vector<std::vector<int>> wordIds;
                    for (const auto& word : words) {
                        auto ids = getSubwordIds(word, dict, ngrams);
                        if (!ids.empty()) {
                            wordEmbeddings.push_back(backbone.forward(ids));
                            wordIds.push_back(ids);
                        }
                    }
                    batchWordEmbeddings.push_back(wordEmbeddings);
                    batchWordIds.push_back(wordIds);
                }
                
                // Process batch when full
                if (static_cast<int>(batchIds.size()) >= args.batchSize) {
                    float loss;
                    
                    if (args.useSentenceEncoder) {
                        // Train with sentence encoding (includes backprop)
                        loss = clf.trainBatchWithSentences(batchWordEmbeddings, batchWordIds, 
                                                          &sentEnc, batchLabels);
                    } else {
                        // Train batch with bag-of-words
                        loss = clf.trainBatch(batchIds, batchLabels);
                    }
                    
                    totalLoss += loss * batchIds.size();
                    processed += batchIds.size();
                    
                    // Clear batch
                    batchIds.clear();
                    batchLabels.clear();
                    batchWordEmbeddings.clear();
                    batchWordIds.clear();
                    
                    if (processed % 1000 == 0 && processed > 0) {
                        std::cout << "\rEpoch " << (epoch + 1) << "/" << args.epoch 
                                 << " - Loss: " << (totalLoss / processed) << std::flush;
                    }
                }
            }
        }
        
        // Process remaining examples in batch
        if (!batchIds.empty()) {
            float loss;
            
            if (args.useSentenceEncoder) {
                loss = clf.trainBatchWithSentences(batchWordEmbeddings, batchWordIds,
                                                   &sentEnc, batchLabels);
            } else {
                loss = clf.trainBatch(batchIds, batchLabels);
            }
            
            totalLoss += loss * batchIds.size();
            processed += batchIds.size();
        }
        
        float avgLoss = processed > 0 ? (totalLoss / processed) : 0.0f;
        std::cout << "\rEpoch " << (epoch + 1) << "/" << args.epoch 
                  << " - Loss: " << avgLoss;
        
        // Validation evaluation
        if (useValidation) {
            ValidationMetrics metrics = evaluateValidation(
                args.validFile, dict, ngrams, backbone, sentEnc, clf, 
                label2id, args.useSentenceEncoder
            );
            
            std::cout << " | Val Acc: " << (metrics.accuracy * 100) << "%"
                      << " P: " << (metrics.precision * 100) << "%"
                      << " R: " << (metrics.recall * 100) << "%"
                      << " F1: " << (metrics.f1 * 100) << "%"
                      << " (" << metrics.correct << "/" << metrics.total << ")";
            
            // Check for improvement
            if (metrics.accuracy > bestAccuracy) {
                bestAccuracy = metrics.accuracy;
                patienceCounter = 0;
                std::cout << " [BEST]";
                
                // Save best model (we'll implement this after the loop)
                // For now, just track that this is the best epoch
            } else {
                patienceCounter++;
                
                // Early stopping check
                if (args.earlyStop && patienceCounter >= args.patience) {
                    std::cout << "\n";
                    std::cout << "Early stopping triggered after " << (epoch + 1) 
                              << " epochs (patience: " << args.patience << ")\n";
                    std::cout << "Best validation accuracy: " << (bestAccuracy * 100) << "%\n";
                    break;
                }
            }
        }
        
        std::cout << "\n";
    }
    
    // Save model
    std::ofstream out(args.output + ".bin", std::ios::binary);
    
    // Save metadata with version
    int magic = 0x47414C53; // "GALS" (supervised)
    int version = 2;  // Version 2: includes ModelConfig
    out.write((char*)&magic, sizeof(magic));
    out.write((char*)&version, sizeof(version));
    
    // Save ModelConfig
    ModelConfig modelConfig;
    modelConfig.dim = dim;
    modelConfig.bucket_size = bucket;
    modelConfig.minn = minn;
    modelConfig.maxn = maxn;
    modelConfig.use_char_ngrams = true;
    modelConfig.use_grammar_units = args.useGrammarUnits;
    modelConfig.use_phonetic = args.usePhonetic;
    modelConfig.use_sentence_encoder = args.useSentenceEncoder;
    saveModelConfig(out, modelConfig);
    
    // Save legacy parameters for backward compatibility
    out.write((char*)&dim, sizeof(dim));
    out.write((char*)&minn, sizeof(minn));
    out.write((char*)&maxn, sizeof(maxn));
    out.write((char*)&bucket, sizeof(bucket));
    
    // Save sentence encoder flag (legacy)
    int sentenceFlag = args.useSentenceEncoder ? 1 : 0;
    out.write((char*)&sentenceFlag, sizeof(sentenceFlag));
    
    // Save attention mode
    int attModeInt = static_cast<int>(att->mode);
    out.write((char*)&attModeInt, sizeof(attModeInt));
    
    // Save dictionary
    int nwords = dict.id2word.size();
    out.write((char*)&nwords, sizeof(nwords));
    for (const auto& word : dict.id2word) {
        int len = word.size();
        out.write((char*)&len, sizeof(len));
        out.write(word.c_str(), len);
    }
    
    // Save grammar units
    int ngrammar = dict.grammar2id.size();
    out.write((char*)&ngrammar, sizeof(ngrammar));
    for (const auto& kv : dict.grammar2id) {
        int len = kv.first.size();
        out.write((char*)&len, sizeof(len));
        out.write(kv.first.c_str(), len);
        out.write((char*)&kv.second, sizeof(kv.second));
    }
    
    // Save phonetic units
    int nphonetic = dict.phonetic2id.size();
    out.write((char*)&nphonetic, sizeof(nphonetic));
    for (const auto& kv : dict.phonetic2id) {
        int len = kv.first.size();
        out.write((char*)&len, sizeof(len));
        out.write(kv.first.c_str(), len);
        out.write((char*)&kv.second, sizeof(kv.second));
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
    
    // Cleanup - FIXED: Always delete allocated memory
    delete emb;
    delete att;
    // sentEnc destructor handles its own cleanup
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
