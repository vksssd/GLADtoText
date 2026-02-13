# GLADtoTEXT Project Structure

## Directory Layout

```
GLADtoTEXT/
├── README.md                    # Project overview and quick start
├── GUIDE.md                     # Complete training and usage guide
├── PYTHON_GUIDE.md              # Python bindings guide
├── CONTRIBUTING.md              # Development guidelines
├── PROJECT_STRUCTURE.md         # This file
│
├── Makefile                     # Build system
├── .gitignore                   # Git ignore rules
│
├── main.cpp                     # Training CLI
├── inference.cpp                # Inference CLI
├── compact_model.cpp            # Compact model training
├── compact_infer.cpp            # Compact model inference
├── tiny_model.cpp               # Tiny model training
├── model_size_calculator.cpp    # Model size calculator
│
├── python_bindings.cpp          # Python bindings (pybind11)
├── setup.py                     # Python package setup
├── pyproject.toml               # Python build config
├── requirements.txt             # Python dependencies
├── test_python_bindings.sh      # Python test script
│
├── core/                        # Core data structures
│   ├── config.h                 # Configuration
│   ├── model_config.h           # Model configuration
│   ├── dictionary.h             # Vocabulary management
│   ├── vector.h                 # Vector operations
│   ├── matrix.h                 # Matrix operations
│   └── sparse_matrix.h          # Sparse matrix
│
├── model/                       # Neural network components
│   ├── embeddings.h             # Word embeddings
│   ├── attention.h              # Vector attention
│   ├── backbone.h               # Feature combination
│   ├── sentence_encoder.h       # Sentence-level encoding
│   ├── memory.h                 # Context memory
│   └── tiny_embeddings.h        # Compact embeddings
│
├── subwords/                    # Subword features
│   ├── char_ngram.h             # Character n-grams
│   ├── grammar_units.h          # Grammar units
│   └── phonetic.h               # Phonetic encoding
│
├── heads/                       # Task-specific heads
│   ├── classify.h               # Classification head
│   └── semantic.h               # Semantic learning head
│
├── training/                    # Training implementations
│   ├── supervised.cpp           # Supervised training
│   └── unsupervised.cpp         # Unsupervised training
│
├── tests/                       # Test suite
│   ├── README.md                # Test documentation
│   ├── unit/                    # Unit tests
│   │   ├── test_backbone.cpp
│   │   ├── test_dictionary.cpp
│   │   ├── test_unsupervised.cpp
│   │   ├── test_supervised.cpp
│   │   ├── test_memory.cpp
│   │   ├── test_char_ngrams.cpp
│   │   ├── test_grammar.cpp
│   │   └── test_grammar2.cpp
│   ├── integration/             # Integration tests
│   │   ├── test_full_pipeline.sh
│   │   ├── test_sentence_encoding.sh
│   │   └── test_transfer_learning.sh
│   ├── comprehensive/           # Comprehensive tests
│   │   ├── README.md
│   │   ├── run_all_tests.sh
│   │   ├── test_intent_classification.sh
│   │   ├── test_sentiment_classification.sh
│   │   └── test_topic_classification.sh
│   ├── stress/                  # Stress tests
│   │   ├── README.md
│   │   ├── run_stress_tests.sh
│   │   ├── test_large_scale.sh
│   │   ├── test_edge_cases.sh
│   │   ├── test_robustness.sh
│   │   └── test_stress.sh
│   └── data/                    # Test datasets
│       ├── intent_classification.txt
│       ├── intent_validation.txt
│       ├── sentiment_large.txt
│       ├── sentiment_validation.txt
│       ├── topic_classification.txt
│       ├── topic_validation.txt
│       ├── edge_cases.txt
│       ├── large_scale_train.txt
│       └── large_scale_validation.txt
│
├── examples/                    # Working examples
│   ├── README.md
│   ├── quickstart.sh
│   ├── demo_compact.sh
│   ├── demo_sentence_encoding.sh
│   ├── example_intent_classification.sh
│   ├── example_transfer_learning.sh
│   └── test_transfer_learning.sh
│
├── python/                      # Python package
│   ├── README.md                # Python API reference
│   └── example_usage.py         # Python example
│
└── docs/                        # Technical documentation
    ├── README.md                # Documentation index
    ├── ARCHITECTURE.md          # System architecture
    ├── STATUS.md                # Project status
    ├── ROADMAP.md               # Development roadmap
    ├── CHALLENGES_AND_GAPS.md   # Known limitations
    ├── COMPREHENSIVE_TEST_SUITE.md
    ├── STRESS_TEST_SUITE.md
    └── LARGE_SCALE_TEST_COMPLETION.md
```

## Key Files

### Build & Configuration
- **Makefile** - Build system with targets for all binaries and tests
- **.gitignore** - Excludes binaries, models, and temporary files
- **setup.py** - Python package configuration
- **pyproject.toml** - Modern Python build configuration

### Documentation
- **README.md** - Project overview, features, quick start
- **GUIDE.md** - Complete guide with all features and examples
- **PYTHON_GUIDE.md** - Python usage, API, examples, datasets
- **CONTRIBUTING.md** - Development guidelines and workflow
- **PROJECT_STRUCTURE.md** - This file

### Core Implementation
- **main.cpp** - Training CLI (CBOW and supervised)
- **inference.cpp** - Inference CLI (predict, word vectors)
- **python_bindings.cpp** - Python bindings using pybind11

### Model Variants
- **compact_model.cpp** - Ultra-small models (4-20KB)
- **tiny_model.cpp** - Small models with all features (80-200KB)
- **model_size_calculator.cpp** - Calculate model sizes

## Build Targets

### Main Binaries
```bash
make all                  # Build training and inference
make compact              # Build compact model tools
make tiny                 # Build tiny model tools
make tools                # Build utilities
```

### Testing
```bash
make test                 # Unit + integration tests
make test-unit            # Unit tests only
make test-integration     # Integration tests only
make test-comprehensive   # Comprehensive test suite
make test-stress          # Stress test suite
make test-all             # All tests
```

### Python
```bash
make python               # Build Python bindings
make python-install       # Install Python package
make python-test          # Test Python bindings
make python-dev           # Development install
```

### Examples
```bash
make example-quickstart   # Quick demo
make example-intent       # Intent classification
make example-transfer     # Transfer learning
make example-compact      # Compact models
make example-sentence     # Sentence encoding
```

### Cleanup
```bash
make clean                # Remove all build artifacts
```

## Module Organization

### Core (`core/`)
Low-level data structures and utilities:
- Configuration management
- Dictionary and vocabulary
- Vector and matrix operations
- Sparse matrix for memory efficiency

### Model (`model/`)
Neural network components:
- Word embeddings (dense and sparse)
- Vector attention mechanism
- Backbone for feature combination
- Sentence encoder with self-attention
- Context memory for sequential tasks

### Subwords (`subwords/`)
Subword feature extraction:
- Character n-grams with FNV-1a hashing
- Grammar units (frequent substrings)
- Phonetic encoding (sound-based)

### Heads (`heads/`)
Task-specific output layers:
- Classification head for supervised learning
- Semantic head for unsupervised learning (CBOW, Skip-gram)

### Training (`training/`)
Training implementations:
- Supervised training with validation
- Unsupervised training (CBOW)

## Data Flow

### Training Flow
```
Input Text
    ↓
Dictionary (tokenization)
    ↓
Subword Features (char n-grams, grammar, phonetic)
    ↓
Embeddings (word + subword)
    ↓
Vector Attention (feature weighting)
    ↓
[Optional] Sentence Encoder (self-attention)
    ↓
Backbone (combined representation)
    ↓
Classification Head
    ↓
Loss & Backpropagation
```

### Inference Flow
```
Input Text
    ↓
Dictionary (tokenization)
    ↓
Subword Features
    ↓
Embeddings
    ↓
Vector Attention
    ↓
[Optional] Sentence Encoder
    ↓
Backbone
    ↓
Classification Head
    ↓
Predictions (label + score)
```

## File Naming Conventions

### Source Files
- `*.cpp` - C++ implementation files
- `*.h` - C++ header files (header-only implementation)

### Test Files
- `test_*.cpp` - Unit test source files
- `test_*.sh` - Integration/system test scripts
- `*_test.sh` - Alternative test script naming

### Data Files
- `*_train.txt` - Training data
- `*_validation.txt` - Validation data
- `*.bin` - Binary model files
- `*.config` - Configuration files

### Documentation
- `README.md` - Overview and guide
- `*_GUIDE.md` - Detailed guides
- `*.md` - General documentation

## Dependencies

### Build Dependencies
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Make

### Python Dependencies
- pybind11 >= 2.10.0
- numpy >= 1.19.0

### Optional Dependencies
- scikit-learn (for clustering examples)

## Model File Format

### Binary Format
```
Magic Number (4 bytes)      # 0x47414C44 (CBOW) or 0x47414C53 (Supervised)
Version (4 bytes)           # Format version
ModelConfig (variable)      # Layer configuration
Parameters (variable)       # dim, minn, maxn, bucket
Dictionary (variable)       # Word vocabulary
Grammar Units (variable)    # Grammar unit vocabulary
Phonetic Units (variable)   # Phonetic vocabulary
Labels (variable)           # Label vocabulary (supervised only)
Embeddings (variable)       # Embedding matrix
Attention (variable)        # Attention weights
Prototypes (variable)       # Classification prototypes (supervised only)
Sentence Encoder (variable) # Attention matrices (if enabled)
```

## Testing Strategy

### Unit Tests
- Test individual components in isolation
- Fast execution (< 1 second each)
- No external dependencies

### Integration Tests
- Test complete workflows
- Train small models and verify output
- Test transfer learning pipeline

### Comprehensive Tests
- Larger datasets (60-100 examples)
- Multiple classification scenarios
- Realistic data variations

### Stress Tests
- Large-scale datasets (10,000+ examples)
- Edge cases (typos, negations, special chars)
- Performance and memory testing

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code style guidelines
- Testing requirements
- Pull request process
- Development workflow

## License

MIT License - See LICENSE file for details
