#pragma once

#include <cstdint>
#include <string_view>

class HashFunction {
public:
    // FNV-1a hash algorithm
    static uint64_t fnv1a(std::string_view str) {
        constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        
        uint64_t hash = FNV_OFFSET;
        for (char c : str) {
            hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
            hash *= FNV_PRIME;
        }
        return hash;
    }
    
    // MurmurHash3 64-bit
    static uint64_t murmur3(std::string_view str, uint64_t seed = 0) {
        const uint64_t m = 0xc6a4a7935bd1e995ULL;
        const int r = 47;
        
        uint64_t h = seed ^ (str.size() * m);
        
        const uint64_t* data = reinterpret_cast<const uint64_t*>(str.data());
        const uint64_t* end = data + (str.size() / 8);
        
        while (data != end) {
            uint64_t k = *data++;
            k *= m;
            k ^= k >> r;
            k *= m;
            h ^= k;
            h *= m;
        }
        
        const unsigned char* data2 = reinterpret_cast<const unsigned char*>(data);
        switch (str.size() & 7) {
            case 7: h ^= uint64_t(data2[6]) << 48; [[fallthrough]];
            case 6: h ^= uint64_t(data2[5]) << 40; [[fallthrough]];
            case 5: h ^= uint64_t(data2[4]) << 32; [[fallthrough]];
            case 4: h ^= uint64_t(data2[3]) << 24; [[fallthrough]];
            case 3: h ^= uint64_t(data2[2]) << 16; [[fallthrough]];
            case 2: h ^= uint64_t(data2[1]) << 8;  [[fallthrough]];
            case 1: h ^= uint64_t(data2[0]);
                    h *= m;
        }
        
        h ^= h >> r;
        h *= m;
        h ^= h >> r;
        
        return h;
    }
};
