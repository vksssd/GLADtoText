#include "rng.h"

RNG::RNG(uint64_t seed): engine_(seed) {}

float RNG::uniform(float a, float b){
    std::uniform_real_distribution<float> dist(a,b);
    return dist(engine_);
}

float RNG::normal(float mean, float stddev){
    std::normal_distribution<float> dist(mean, stddev);
    return dist(engine_);
}

