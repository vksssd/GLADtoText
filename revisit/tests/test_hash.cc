#include <gtest/gtest.h>
#include "hashing/hash_function.h"

TEST(HashFunctionTest, FNV1aDeterministic) {
    uint64_t h1 = HashFunction::fnv1a("hello");
    uint64_t h2 = HashFunction::fnv1a("hello");
    
    EXPECT_EQ(h1, h2);
}

TEST(HashFunctionTest, FNV1aDifferent) {
    uint64_t h1 = HashFunction::fnv1a("hello");
    uint64_t h2 = HashFunction::fnv1a("world");
    
    EXPECT_NE(h1, h2);
}

TEST(HashFunctionTest, MurmurDeterministic) {
    uint64_t h1 = HashFunction::murmur3("test", 42);
    uint64_t h2 = HashFunction::murmur3("test", 42);
    
    EXPECT_EQ(h1, h2);
}

TEST(HashFunctionTest, EmptyString) {
    uint64_t h = HashFunction::fnv1a("");
    EXPECT_NE(h, 0);
}
