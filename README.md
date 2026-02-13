# GLADtoTEXT Revisit - Modern C++ Implementation

Clean, modular rewrite of GLADtoTEXT with modern C++ practices, comprehensive testing, and CMake build system.

## Features

- ✅ **Modern C++17**: Clean interfaces, RAII, proper const-correctness
- ✅ **CMake Build System**: Cross-platform, IDE-friendly
- ✅ **Comprehensive Tests**: GoogleTest framework with 18+ unit tests
- ✅ **Memory Efficient**: Aligned allocations for SIMD, proper resource management
- ✅ **Deterministic**: Seeded RNG for reproducible experiments
- ✅ **Modular Design**: Clear separation of concerns

## Components

### Core
- **ModelConfig**: Centralized configuration (no feature flags)
- **EmbeddingTable**: Hash-based embedding storage with aligned memory
- **WordEncoder**: N-gram + phonetic encoding
- **NGramGenerator**: Character n-gram extraction
- **PhoneticEncoder**: Soundex-like phonetic encoding
- **HashFunction**: FNV-1a and MurmurHash3 implementations

### Utils
- **RNG**: Deterministic random number generation (MT19937-64)
- **Logger**: Thread-safe logging with levels
- **AlignedAlloc**: SIMD-friendly memory allocation

### Tokenization
- **EnglishTokenizer**: Simple whitespace + punctuation tokenizer

## Building

```bash
# Run tests
./build_and_test.sh

# Or manually
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure

# Run example
./build/example_usage
```

## Running Tests

```bash
# All tests
./build/gladtotext_tests

# Specific test
./build/gladtotext_tests --gtest_filter=EmbeddingTableTest.*

# List tests
./build/gladtotext_tests --gtest_list_tests
```

## Test Coverage

- ✅ ModelConfig: defaults, equality, validation
- ✅ RNG: deterministic generation
- ✅ EmbeddingTable: construction, access, determinism
- ✅ NGramGenerator: generation, correctness
- ✅ HashFunction: FNV-1a, MurmurHash3
- ✅ Tokenizer: basic, punctuation, edge cases
- ✅ LinearClassifier: forward, backward, determinism
- ✅ Softmax & CrossEntropy: numerical stability, correctness
- ✅ WordEncoder: encoding, determinism, phonetic contribution
- ✅ MeanSentenceEncoder: averaging, empty handling, determinism
- ✅ PhoneticEncoder: soundex, case handling, edge cases
- ✅ Training: overfitting, determinism, convergence
- ✅ Edge Cases: long inputs, special chars, unicode, extreme values
- ✅ Integration: end-to-end pipeline, training reduces loss

Total: 79 tests across 15 test suites

## Example Usage

```cpp
#include "config/model_config.h"
#include "embedding/embedding_table.h"
#include "word_ecoder/word_encoder.h"

// Create config
ModelConfig config;

// Create embeddings
EmbeddingTable embeddings(config.bucket_count, 
                         config.embedding_dim, 
                         config.seed);

// Create encoder
NGramGenerator ngram(config.ngram_min, config.ngram_max);
PhoneticEncoder phonetic;
WordEncoder encoder(embeddings, ngram, &phonetic, 
                   config.bucket_count, config.phonetic_gamma);

// Encode word
std::vector<float> embedding(config.embedding_dim);
encoder.encode("hello", embedding.data());
```

## Architecture

```
core/
├── config/          # Configuration
├── embedding/       # Embedding storage
├── word_ecoder/     # Word encoding logic
├── ngram/           # N-gram generation
├── phonetic/        # Phonetic encoding
├── hashing/         # Hash functions
├── tokenizer/       # Text tokenization
└── utils/           # Utilities (RNG, logger, memory)

tests/               # Unit tests
external/            # GoogleTest (auto-downloaded)
```

## Key Improvements Over Original

1. **Simpler Config**: 14 parameters vs 20+ boolean flags
2. **Better Testing**: Unit tests vs shell scripts only
3. **Modern Build**: CMake vs basic Makefile
4. **Cleaner Code**: Separation of concerns, RAII
5. **Performance**: Aligned memory, scratch buffers
6. **Reproducibility**: Seeded RNG throughout

## Memory Usage

Example with default config:
- Embedding table: ~195 MB (200k buckets × 256 dim)
- Per-word encoding: ~2 KB scratch space
- Total: Configurable via `bucket_count` and `embedding_dim`

## Configuration

```cpp
ModelConfig config;
config.embedding_dim = 128;      // Smaller embeddings
config.bucket_count = 100000;    // Fewer buckets
config.ngram_min = 2;            // Shorter n-grams
config.ngram_max = 5;
config.phonetic_gamma = 0.1f;    // Less phonetic weight
config.seed = 42;                // Reproducibility
```

## Next Steps

- [ ] Add attention mechanism
- [ ] Add training loop
- [ ] Add classification head
- [ ] Add model serialization
- [ ] Add Python bindings
- [ ] Add benchmarks

## Comparison with Main Codebase

| Feature | Main | Revisit |
|---------|------|---------|
| Build System | Makefile | CMake |
| Testing | Shell scripts | GoogleTest |
| Config | 20+ flags | 14 parameters |
| Code Style | Header-only | Header + impl |
| Memory | Manual | RAII + aligned |
| Reproducibility | Limited | Full (seeded) |

## License

Same as main GLADtoTEXT project (MIT).
