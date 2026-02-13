#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <sstream>
#include "core/config.h"
#include "core/model_config.h"
#include "core/dictionary.h"
#include "subwords/char_ngram.h"
#include "subwords/phonetic.h"
#include "model/embeddings.h"
#include "model/attention.h"
#include "model/backbone.h"
#include "model/sentence_encoder.h"
#include "heads/classify.h"

namespace py = pybind11;

// Load ModelConfig from binary stream
void loadModelConfig(std::ifstream& in, ModelConfig& config) {
    in.read((char*)&config.use_word_embeddings, sizeof(bool));
    in.read((char*)&config.use_char_ngrams, sizeof(bool));
    in.read((char*)&config.use_grammar_units, sizeof(bool));
    in.read((char*)&config.use_phonetic, sizeof(bool));
    in.read((char*)&config.use_vector_attention, sizeof(bool));
    in.read((char*)&config.use_sentence_encoder, sizeof(bool));
    in.read((char*)&config.dim, sizeof(int));
    in.read((char*)&config.bucket_size, sizeof(int));
    in.read((char*)&config.minn, sizeof(int));
    in.read((char*)&config.maxn, sizeof(int));
}

class GLADtoTEXT {
private:
    int dim;
    int minn, maxn, bucket;
    Dictionary dict;
    Embeddings* emb = nullptr;
    VectorAttention* att = nullptr;
    Backbone* backbone = nullptr;
    SentenceEncoder* sentEnc = nullptr;
    Classifier* clf = nullptr;
    std::vector<std::string> labels;
    bool supervised = false;
    bool useSentenceEncoder = false;
    AttentionMode attentionMode = AttentionMode::FULL;
    
public:
    ~GLADtoTEXT() {
        delete clf;
        delete sentEnc;
        delete backbone;
        delete att;
        delete emb;
    }
    
    bool load_model(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) return false;
        
        int magic;
        in.read((char*)&magic, sizeof(magic));
        
        if (magic == 0x47414C53) {
            supervised = true;
        } else if (magic != 0x47414C44) {
            return false;
        }
        
        int version = 1;
        int firstInt;
        in.read((char*)&firstInt, sizeof(firstInt));
        
        if (firstInt >= 1 && firstInt <= 10) {
            version = firstInt;
            if (version >= 2) {
                ModelConfig config;
                loadModelConfig(in, config);
            }
            in.read((char*)&dim, sizeof(dim));
            in.read((char*)&minn, sizeof(minn));
            in.read((char*)&maxn, sizeof(maxn));
            in.read((char*)&bucket, sizeof(bucket));
        } else {
            dim = firstInt;
            in.read((char*)&minn, sizeof(minn));
            in.read((char*)&maxn, sizeof(maxn));
            in.read((char*)&bucket, sizeof(bucket));
        }
        
        if (supervised) {
            int sentenceFlag = 0;
            in.read((char*)&sentenceFlag, sizeof(sentenceFlag));
            useSentenceEncoder = (sentenceFlag == 1);
            
            if (version >= 2) {
                int attModeInt = 0;
                in.read((char*)&attModeInt, sizeof(attModeInt));
                attentionMode = static_cast<AttentionMode>(attModeInt);
            }
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
        
        // Read grammar units
        int ngrammar;
        in.read((char*)&ngrammar, sizeof(ngrammar));
        std::unordered_map<std::string, int> grammarVocab;
        for (int i = 0; i < ngrammar; i++) {
            int len;
            in.read((char*)&len, sizeof(len));
            std::string unit(len, ' ');
            in.read(&unit[0], len);
            int id;
            in.read((char*)&id, sizeof(id));
            grammarVocab[unit] = id;
        }
        if (!grammarVocab.empty()) {
            dict.loadGrammarUnits(grammarVocab);
        }
        
        // Read phonetic units
        int nphonetic;
        in.read((char*)&nphonetic, sizeof(nphonetic));
        for (int i = 0; i < nphonetic; i++) {
            int len;
            in.read((char*)&len, sizeof(len));
            std::string phonetic(len, ' ');
            in.read(&phonetic[0], len);
            int id;
            in.read((char*)&id, sizeof(id));
            dict.addPhonetic(phonetic);
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
        att = new VectorAttention(dim, attentionMode);
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
            
            if (useSentenceEncoder) {
                sentEnc = new SentenceEncoder(dim, true);
                int attDim;
                in.read((char*)&attDim, sizeof(attDim));
                
                for (int i = 0; i < attDim; i++) {
                    for (int j = 0; j < attDim; j++) {
                        float val;
                        in.read((char*)&val, sizeof(val));
                        sentEnc->attention->query_w.set(i, j, val);
                    }
                }
                
                for (int i = 0; i < attDim; i++) {
                    for (int j = 0; j < attDim; j++) {
                        float val;
                        in.read((char*)&val, sizeof(val));
                        sentEnc->attention->key_w.set(i, j, val);
                    }
                }
                
                for (int i = 0; i < attDim; i++) {
                    for (int j = 0; j < attDim; j++) {
                        float val;
                        in.read((char*)&val, sizeof(val));
                        sentEnc->attention->value_w.set(i, j, val);
                    }
                }
            }
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
        
        for (const auto& kv : dict.grammar2id) {
            if (word.find(kv.first) != std::string::npos) {
                ids.push_back(kv.second);
            }
        }
        
        std::string phonetic = phoneticEncode(word);
        int pid = dict.getPhoneticId(phonetic);
        if (pid >= 0) ids.push_back(pid);
        
        return ids;
    }
    
    py::array_t<float> get_word_vector(const std::string& word) {
        auto ids = getSubwordIds(word);
        Vector v = backbone->forward(ids);
        
        py::array_t<float> result(v.v.size());
        auto buf = result.request();
        float* ptr = static_cast<float*>(buf.ptr);
        
        for (size_t i = 0; i < v.v.size(); i++) {
            ptr[i] = v.v[i];
        }
        
        return result;
    }
    
    py::list predict(const std::string& text, int k = 1) {
        if (!supervised) {
            throw std::runtime_error("Model is not supervised");
        }
        
        std::istringstream iss(text);
        std::string word;
        std::vector<int> textIds;
        std::vector<std::string> words;
        
        while (iss >> word) {
            words.push_back(word);
            auto ids = getSubwordIds(word);
            textIds.insert(textIds.end(), ids.begin(), ids.end());
        }
        
        if (textIds.empty()) return py::list();
        
        Vector h;
        if (useSentenceEncoder) {
            std::vector<Vector> wordEmbeddings;
            for (const auto& w : words) {
                auto ids = getSubwordIds(w);
                if (!ids.empty()) {
                    wordEmbeddings.push_back(backbone->forward(ids));
                }
            }
            h = sentEnc->encode(wordEmbeddings);
        } else {
            h = backbone->forward(textIds);
        }
        
        std::vector<std::pair<std::string, float>> results;
        for (size_t i = 0; i < labels.size(); i++) {
            float score = clf->prototypes.row(i).dot(h);
            results.push_back({labels[i], score});
        }
        
        std::sort(results.begin(), results.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (k < (int)results.size()) {
            results.resize(k);
        }
        
        py::list pyResults;
        for (const auto& r : results) {
            pyResults.append(py::make_tuple(r.first, r.second));
        }
        
        return pyResults;
    }
    
    py::dict get_info() {
        py::dict info;
        info["vocab_size"] = dict.id2word.size();
        info["dim"] = dim;
        info["supervised"] = supervised;
        info["sentence_encoder"] = useSentenceEncoder;
        if (supervised) {
            info["num_labels"] = labels.size();
            py::list label_list;
            for (const auto& label : labels) {
                label_list.append(label);
            }
            info["labels"] = label_list;
        }
        return info;
    }
};

PYBIND11_MODULE(gladtotext, m) {
    m.doc() = "GLADtoTEXT Python bindings - FastText-inspired text embeddings";
    
    py::class_<GLADtoTEXT>(m, "Model")
        .def(py::init<>())
        .def("load_model", &GLADtoTEXT::load_model, 
             "Load a trained model from file",
             py::arg("filename"))
        .def("get_word_vector", &GLADtoTEXT::get_word_vector,
             "Get word embedding vector",
             py::arg("word"))
        .def("predict", &GLADtoTEXT::predict,
             "Predict label for text",
             py::arg("text"), py::arg("k") = 1)
        .def("get_info", &GLADtoTEXT::get_info,
             "Get model information");
}
