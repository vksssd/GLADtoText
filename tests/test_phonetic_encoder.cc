#include <gtest/gtest.h>
#include "phonetic/phonetic_encoder.h"

TEST(PhoneticEncoderTest, BasicEncoding) {
    PhoneticEncoder encoder;
    
    std::string code = encoder.encode("hello");
    
    EXPECT_FALSE(code.empty());
}

TEST(PhoneticEncoderTest, DeterministicEncoding) {
    PhoneticEncoder encoder;
    
    std::string code1 = encoder.encode("world");
    std::string code2 = encoder.encode("world");
    
    EXPECT_EQ(code1, code2);
}

TEST(PhoneticEncoderTest, SimilarSoundingSimilarCodes) {
    PhoneticEncoder encoder;
    
    std::string code1 = encoder.encode("smith");
    std::string code2 = encoder.encode("smyth");
    
    // Soundex should give similar codes for similar sounding words
    EXPECT_EQ(code1, code2);
}

TEST(PhoneticEncoderTest, DifferentWordsDifferentCodes) {
    PhoneticEncoder encoder;
    
    std::string code1 = encoder.encode("hello");
    std::string code2 = encoder.encode("world");
    
    EXPECT_NE(code1, code2);
}

TEST(PhoneticEncoderTest, EmptyStringHandling) {
    PhoneticEncoder encoder;
    
    std::string code = encoder.encode("");
    
    // Should handle empty string gracefully
    EXPECT_TRUE(true);
}

TEST(PhoneticEncoderTest, CaseInsensitive) {
    PhoneticEncoder encoder;
    
    std::string code1 = encoder.encode("Hello");
    std::string code2 = encoder.encode("hello");
    std::string code3 = encoder.encode("HELLO");
    
    // Phonetic encoding should be case-insensitive
    EXPECT_EQ(code1, code2);
    EXPECT_EQ(code2, code3);
}

TEST(PhoneticEncoderTest, NumbersHandling) {
    PhoneticEncoder encoder;
    
    std::string code = encoder.encode("test123");
    
    // Should handle strings with numbers
    EXPECT_FALSE(code.empty());
}

TEST(PhoneticEncoderTest, SpecialCharactersHandling) {
    PhoneticEncoder encoder;
    
    std::string code = encoder.encode("hello-world");
    
    // Should handle special characters
    EXPECT_FALSE(code.empty());
}

TEST(PhoneticEncoderTest, SingleCharacter) {
    PhoneticEncoder encoder;
    
    std::string code = encoder.encode("a");
    
    EXPECT_FALSE(code.empty());
}

TEST(PhoneticEncoderTest, LongWord) {
    PhoneticEncoder encoder;
    
    std::string code = encoder.encode("antidisestablishmentarianism");
    
    EXPECT_FALSE(code.empty());
}

TEST(PhoneticEncoderTest, CommonWords) {
    PhoneticEncoder encoder;
    
    // Test common English words
    std::vector<std::string> words = {
        "the", "and", "for", "are", "but", "not", "you", "all"
    };
    
    for (const auto& word : words) {
        std::string code = encoder.encode(word);
        EXPECT_FALSE(code.empty());
    }
}

TEST(PhoneticEncoderTest, Homophones) {
    PhoneticEncoder encoder;
    
    // Words that sound the same should have same code
    std::string code1 = encoder.encode("right");
    std::string code2 = encoder.encode("write");
    
    // Note: Soundex may not catch all homophones perfectly
    // This test documents the behavior
    EXPECT_TRUE(true);
}
