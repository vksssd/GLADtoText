#pragma once
#include <vector>
#include <random>

// Forward declaration - Vector is defined in vector.h
class Vector; 

class Matrix {
public:
    int rows, cols;
    std::vector<float> w;

    Matrix(int r, int c) : rows(r), cols(c), w(r * c) {
        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
        for (float& x : w) x = dist(gen);
    }

    Vector row(int i) const;
    void addRow(const Vector& grad, int i, float lr);
};