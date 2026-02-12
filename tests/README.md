# GLADtoTEXT Tests

This directory contains unit and integration tests for GLADtoTEXT.

## Test Structure

```
tests/
├── unit/              # Unit tests for individual components
│   ├── test_backbone.cpp
│   ├── test_dictionary.cpp
│   ├── test_unsupervised.cpp
│   ├── test_supervised.cpp
│   ├── test_memory.cpp
│   ├── test_char_ngrams.cpp
│   ├── test_grammar.cpp
│   └── test_grammar2.cpp
├── integration/       # Integration tests for full workflows
│   ├── test_full_pipeline.sh
│   ├── test_sentence_encoding.sh
│   └── test_transfer_learning.sh
└── README.md         # This file
```

## Running Tests

### All Tests

```bash
# From project root
make test
```

This runs:
1. All unit tests
2. All integration tests

### Unit Tests Only

```bash
make test-unit
```

Tests individual components:
- Vector/Matrix operations
- Dictionary functionality
- Embeddings and attention
- Training algorithms
- Memory management

### Integration Tests Only

```bash
make test-integration
```

Tests complete workflows:
- Full training and inference pipeline
- Sentence encoding
- Transfer learning

## Unit Tests

### test_backbone.cpp
Tests core model components:
- Embeddings lookup
- Attention mechanism
- Forward pass

**Run:**
```bash
g++ -std=c++17 tests/unit/test_backbone.cpp -o tests/unit/t1
./tests/unit/t1
```

**Expected output:**
```
Backbone test:
h1[0] = 0.019811
h2[0] = 0.0345263
PASS
```

### test_dictionary.cpp
Tests vocabulary management:
- Word addition
- ID lookup
- Grammar units
- Phonetic encoding

**Run:**
```bash
g++ -std=c++17 tests/unit/test_dictionary.cpp -o tests/unit/t2
./tests/unit/t2
```

**Expected output:**
```
Word ID: 0
Grammar ID: 1003
PASS
```

### test_unsupervised.cpp
Tests CBOW training:
- Context embedding
- Negative sampling
- Gradient updates

**Run:**
```bash
g++ -std=c++17 tests/unit/test_unsupervised.cpp -o tests/unit/t3
./tests/unit/t3
```

**Expected output:**
```
Context embedding change: -0.0391515 → -0.0440363
PASS
```

### test_supervised.cpp
Tests classification:
- Prototype learning
- Loss calculation
- Prediction

**Run:**
```bash
g++ -std=c++17 tests/unit/test_supervised.cpp -o tests/unit/t4
./tests/unit/t4
```

**Expected output:**
```
Loss: 1.21052 → 0.228374
PASS
```

### test_memory.cpp
Tests context memory:
- Memory storage
- Memory retrieval
- Memory updates

**Run:**
```bash
g++ -std=c++17 tests/unit/test_memory.cpp -o tests/unit/t5
./tests/unit/t5
```

**Expected output:**
```
Memory applied output:
2.217 2.217 2.217 2.217
PASS
```

## Integration Tests

### test_full_pipeline.sh
Tests complete training and inference:
1. Create training data
2. Train model
3. Run inference
4. Verify predictions

**Run:**
```bash
cd tests/integration
./test_full_pipeline.sh
```

**Expected output:**
```
=== Full Pipeline Integration Test ===

1. Training model...
✓ Training successful

2. Testing inference...
✓ Inference successful: __positive 0.123

3. Checking model file...
✓ Model file created: 153M

=== All integration tests passed! ===
```

### test_sentence_encoding.sh
Tests sentence-level encoding:
1. Train with `-sentence` flag
2. Verify sentence encoding is active
3. Test predictions

**Run:**
```bash
cd tests/integration
./test_sentence_encoding.sh
```

**Expected output:**
```
=== Sentence Encoding Integration Test ===

1. Training with sentence encoding...
✓ Training successful

2. Testing inference...
✓ Sentence encoding detected in model
✓ Prediction correct

=== Sentence encoding test passed! ===
```

### test_transfer_learning.sh
Tests transfer learning workflow:
1. Pretrain with CBOW
2. Fine-tune with supervised learning
3. Verify improved performance

**Run:**
```bash
cd tests/integration
./test_transfer_learning.sh
```

**Expected output:**
```
=== Transfer Learning Integration Test ===

1. Pretraining with CBOW...
✓ Pretraining successful

2. Fine-tuning with pretrained embeddings...
✓ Fine-tuning successful

3. Testing inference...
✓ Prediction correct: __positive 0.456

=== Transfer learning test passed! ===
```

## Adding New Tests

### Unit Test Template

```cpp
#include <iostream>
#include "../core/vector.h"

int main() {
    // Setup
    Vector v(10);
    
    // Test
    v.v[0] = 1.0f;
    
    // Verify
    if (v.v[0] == 1.0f) {
        std::cout << "PASS\n";
        return 0;
    } else {
        std::cout << "FAIL\n";
        return 1;
    }
}
```

### Integration Test Template

```bash
#!/bin/bash

echo "=== My Integration Test ==="
echo ""

# Create test data
cat > test.txt << 'EOF'
__label1 example
EOF

# Train
../../gladtotext supervised -input test.txt -output model \
  -dim 20 -epoch 5 -minCount 1 > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Test passed"
else
    echo "✗ Test failed"
    exit 1
fi

# Cleanup
rm -f test.txt model.bin

echo "=== Test complete ==="
```

## Continuous Integration

Tests are designed to be run in CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Run tests
  run: |
    make all
    make test
```

## Test Coverage

Current coverage:
- ✓ Core components (Vector, Matrix, Dictionary)
- ✓ Model components (Embeddings, Attention, Backbone)
- ✓ Training algorithms (CBOW, Supervised)
- ✓ Subword features (Char n-grams, Grammar, Phonetic)
- ✓ Sentence encoding
- ✓ Transfer learning
- ✓ Full pipeline

## Troubleshooting

### Tests fail to compile
```bash
# Rebuild from scratch
make clean
make all
make test-unit
```

### Integration tests fail
```bash
# Check if binaries exist
ls -la gladtotext gladtotext-infer

# Rebuild if needed
make all
```

### Permission denied
```bash
chmod +x tests/integration/*.sh
```

## See Also

- [Makefile](../Makefile) - Build configuration
- [README.md](../README.md) - Main documentation
- [examples/](../examples/) - Example scripts
