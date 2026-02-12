#pragma once
#include <vector>
#include <cmath>

class Vector {
    public:
    std::vector<float> v;

    explicit Vector(int dim=0) : v(dim, 0.0f) {}

    void zero() {
        std::fill(v.begin(), v.end(), 0.0f);
    }

    void add(const Vector& other, float scale = 1.0f){
        for(size_t i=0; i<v.size(); i++){
            v[i]+=scale *other.v[i];
        }
    }

    void scale(float s){
        for(float& x:v)
            x*=s;
    }

    float dot(const Vector& other) const {
        float sum = 0.0f;
        for(size_t i =0; i<v.size();  i++){
            sum += v[i] * other.v[i];
        }
        return sum;
    }

};

// Include matrix header after Vector is defined
#include "matrix.h"

// Matrix method implementations
inline Vector Matrix::row(int i) const {
    Vector v(cols);
    for (int j = 0; j < cols; j++) {
        v.v[j] = w[i * cols + j];
    }
    return v;
}

inline void Matrix::addRow(const Vector& grad, int i, float lr) {
    for (int j = 0; j < cols; j++) {
        w[i * cols + j] += lr * grad.v[j];
    }
}