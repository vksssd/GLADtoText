#pragma once

#include <random>
#include <cstdint>

class RNG {
    public:
        explicit RNG(uint64_t seed);

        float uniform(float a, float b);
        float normal(float mean, float stddev);

    private:
        std::mt19937_64 engine_;
};