cmake_minimum_required(VERSION 3.16)
project(gladtotext LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

enable_testing()

add_library(gladtotext_core
    core/config/model_config.cc
    core/utils/rng.cc
    core/utils/logger.cc
    core/utils/aligned_alloc.cc
    core/embedding/embedding_table.cc
    core/encoder/word_encoder.cc
    core/encoder/mean_sentence_encoder.cc
    core/classifier/linear_classifier.cc
    core/training/simple_trainer.cc
)

target_include_directories(gladtotext_core PUBLIC core)

add_subdirectory(external/googletest)

add_executable(gladtotext_tests
    tests/test_model_config.cc
    tests/test_rng.cc
    tests/test_embedding.cc
    tests/test_ngram.cc
    tests/test_hash.cc
    tests/test_tokenizer.cc
    tests/test_linear_classifier.cc
    tests/test_softmax.cc
    tests/test_training_overfit.cc
    tests/test_training_determinism.cc
    tests/test_word_encoder.cc
    tests/test_mean_sentence_encoder.cc
    tests/test_phonetic_encoder.cc
    tests/test_edge_cases.cc
    tests/test_integration.cc
)

target_link_libraries(gladtotext_tests
    gladtotext_core
    gtest
    gtest_main
)

add_test(NAME gladtotext_tests COMMAND gladtotext_tests)

# Example executable
add_executable(example_usage example_usage.cc)
target_link_libraries(example_usage gladtotext_core)
