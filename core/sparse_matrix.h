#pragma once
#include <unordered_map>
#include <vector>
#include "vector.h"

// Sparse matrix - only stores non-zero values
class SparseMatrix {
public:
    int rows, cols;
    std::unordered_map<int, std::unordered_map<int, float>> data;
    
    SparseMatrix(int r, int c) : rows(r), cols(c) {}
    
    float get(int i, int j) const {
        auto row_it = data.find(i);
        if (row_it == data.end()) return 0.0f;
        auto col_it = row_it->second.find(j);
        if (col_it == row_it->second.end()) return 0.0f;
        return col_it->second;
    }
    
    void set(int i, int j, float val) {
        if (std::abs(val) < 1e-6f) {
            // Remove near-zero values
            auto row_it = data.find(i);
            if (row_it != data.end()) {
                row_it->second.erase(j);
                if (row_it->second.empty()) {
                    data.erase(i);
                }
            }
        } else {
            data[i][j] = val;
        }
    }
    
    void add(int i, int j, float val) {
        float current = get(i, j);
        set(i, j, current + val);
    }
    
    Vector row(int i) const {
        Vector v(cols);
        auto row_it = data.find(i);
        if (row_it != data.end()) {
            for (const auto& kv : row_it->second) {
                v.v[kv.first] = kv.second;
            }
        }
        return v;
    }
    
    void addRow(const Vector& grad, int i, float lr) {
        for (int j = 0; j < cols; j++) {
            if (std::abs(grad.v[j]) > 1e-6f) {
                add(i, j, lr * grad.v[j]);
            }
        }
    }
    
    // Get memory usage in bytes
    size_t memoryUsage() const {
        size_t total = 0;
        for (const auto& row : data) {
            total += row.second.size() * (sizeof(int) + sizeof(float));
        }
        return total;
    }
    
    // Prune small values to save memory
    void prune(float threshold = 1e-4f) {
        std::vector<int> rows_to_remove;
        for (auto& row_pair : data) {
            std::vector<int> cols_to_remove;
            for (auto& col_pair : row_pair.second) {
                if (std::abs(col_pair.second) < threshold) {
                    cols_to_remove.push_back(col_pair.first);
                }
            }
            for (int col : cols_to_remove) {
                row_pair.second.erase(col);
            }
            if (row_pair.second.empty()) {
                rows_to_remove.push_back(row_pair.first);
            }
        }
        for (int row : rows_to_remove) {
            data.erase(row);
        }
    }
};
