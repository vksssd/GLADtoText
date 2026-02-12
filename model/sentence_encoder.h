#pragma once
#include "../core/vector.h"
#include "../core/sparse_matrix.h"
#include <vector>
#include <cmath>

// Position encoding for word order
class PositionEncoder {
public:
    int dim;
    
    explicit PositionEncoder(int d) : dim(d) {}
    
    // Sinusoidal position encoding (Transformer-style)
    // max_len parameter reserved for future dynamic scaling
    Vector encode(int position, int /* max_len */ = 100) const {
        Vector pos(dim);
        for (int i = 0; i < dim; i++) {
            float angle = position / std::pow(10000.0f, 2.0f * i / dim);
            pos.v[i] = (i % 2 == 0) ? std::sin(angle) : std::cos(angle);
        }
        // Scale down to not dominate word embeddings
        pos.scale(0.1f);
        return pos;
    }
};

// Self-attention for sentence encoding
class SentenceAttention {
public:
    SparseMatrix query_w;   // Query projection
    SparseMatrix key_w;     // Key projection
    SparseMatrix value_w;   // Value projection
    int dim;
    
    SentenceAttention(int d) 
        : query_w(d, d), key_w(d, d), value_w(d, d), dim(d) {
        // Initialize with small random values
        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(-0.01f, 0.01f);
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                if (i == j) {  // Identity initialization
                    query_w.set(i, j, 1.0f + dist(gen));
                    key_w.set(i, j, 1.0f + dist(gen));
                    value_w.set(i, j, 1.0f + dist(gen));
                }
            }
        }
    }
    
    // Apply self-attention to sequence of word embeddings
    std::vector<Vector> apply(const std::vector<Vector>& word_embeddings) const {
        int seq_len = word_embeddings.size();
        if (seq_len == 0) return {};
        
        // Compute queries, keys, values
        std::vector<Vector> queries, keys, values;
        for (const auto& emb : word_embeddings) {
            queries.push_back(project(emb, query_w));
            keys.push_back(project(emb, key_w));
            values.push_back(project(emb, value_w));
        }
        
        // Compute attention scores
        std::vector<std::vector<float>> scores(seq_len, std::vector<float>(seq_len));
        float scale = std::sqrt(static_cast<float>(dim));
        
        for (int i = 0; i < seq_len; i++) {
            for (int j = 0; j < seq_len; j++) {
                scores[i][j] = queries[i].dot(keys[j]) / scale;
            }
        }
        
        // Softmax over scores
        for (int i = 0; i < seq_len; i++) {
            float max_score = *std::max_element(scores[i].begin(), scores[i].end());
            float sum = 0.0f;
            for (int j = 0; j < seq_len; j++) {
                scores[i][j] = std::exp(scores[i][j] - max_score);
                sum += scores[i][j];
            }
            for (int j = 0; j < seq_len; j++) {
                scores[i][j] /= sum;
            }
        }
        
        // Apply attention to values
        std::vector<Vector> output;
        for (int i = 0; i < seq_len; i++) {
            Vector out(dim);
            out.zero();
            for (int j = 0; j < seq_len; j++) {
                Vector weighted = values[j];
                weighted.scale(scores[i][j]);
                out.add(weighted);
            }
            output.push_back(out);
        }
        
        return output;
    }
    
    void update(const std::vector<Vector>& word_embeddings,
                const std::vector<Vector>& grad_output,
                float lr) {
        // Simplified gradient update
        // In practice, would compute full backprop through attention
        int seq_len = word_embeddings.size();
        for (int i = 0; i < seq_len; i++) {
            // Update query, key, value projections
            updateProjection(word_embeddings[i], grad_output[i], query_w, lr * 0.1f);
            updateProjection(word_embeddings[i], grad_output[i], key_w, lr * 0.1f);
            updateProjection(word_embeddings[i], grad_output[i], value_w, lr * 0.1f);
        }
    }
    
    void prune(float threshold = 1e-4f) {
        query_w.prune(threshold);
        key_w.prune(threshold);
        value_w.prune(threshold);
    }
    
    size_t memoryUsage() const {
        return query_w.memoryUsage() + key_w.memoryUsage() + value_w.memoryUsage();
    }
    
private:
    Vector project(const Vector& v, const SparseMatrix& w) const {
        Vector result(dim);
        result.zero();
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                result.v[i] += w.get(i, j) * v.v[j];
            }
        }
        return result;
    }
    
    void updateProjection(const Vector& input, const Vector& grad,
                         SparseMatrix& w, float lr) {
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                if (std::abs(input.v[j]) > 1e-6f && std::abs(grad.v[i]) > 1e-6f) {
                    w.add(i, j, lr * grad.v[i] * input.v[j]);
                }
            }
        }
    }
};

// Complete sentence encoder
class SentenceEncoder {
public:
    PositionEncoder* pos_encoder;
    SentenceAttention* attention;
    int dim;
    bool enabled;
    
    SentenceEncoder(int d, bool enable = false) 
        : dim(d), enabled(enable) {
        if (enabled) {
            pos_encoder = new PositionEncoder(d);
            attention = new SentenceAttention(d);
        } else {
            pos_encoder = nullptr;
            attention = nullptr;
        }
    }
    
    ~SentenceEncoder() {
        delete pos_encoder;
        delete attention;
    }
    
    // Encode sentence: word embeddings → sentence embedding
    Vector encode(const std::vector<Vector>& word_embeddings) const {
        if (!enabled || word_embeddings.empty()) {
            // Fallback: simple average (bag-of-words)
            return averagePooling(word_embeddings);
        }
        
        // Add position encoding
        std::vector<Vector> positioned;
        for (size_t i = 0; i < word_embeddings.size(); i++) {
            Vector pos_emb = word_embeddings[i];
            Vector pos_enc = pos_encoder->encode(i);
            pos_emb.add(pos_enc);  // Add position information
            positioned.push_back(pos_emb);
        }
        
        // Apply self-attention
        auto attended = attention->apply(positioned);
        
        // Pool to single vector (mean pooling over attended sequence)
        return averagePooling(attended);
    }
    
    void update(const std::vector<Vector>& word_embeddings,
                const Vector& grad_sentence,
                float lr) {
        if (!enabled || word_embeddings.empty()) return;
        
        // Distribute gradient to all positions
        std::vector<Vector> grad_output;
        for (size_t i = 0; i < word_embeddings.size(); i++) {
            grad_output.push_back(grad_sentence);
        }
        
        // Update attention weights
        attention->update(word_embeddings, grad_output, lr);
    }
    
    void prune(float threshold = 1e-4f) {
        if (enabled && attention) {
            attention->prune(threshold);
        }
    }
    
    size_t memoryUsage() const {
        if (!enabled) return 0;
        return attention ? attention->memoryUsage() : 0;
    }
    
private:
    Vector averagePooling(const std::vector<Vector>& embeddings) const {
        if (embeddings.empty()) {
            return Vector(dim);
        }
        
        Vector result(dim);
        result.zero();
        for (const auto& emb : embeddings) {
            result.add(emb);
        }
        result.scale(1.0f / embeddings.size());
        return result;
    }
};
