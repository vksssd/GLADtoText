# GLADtoTEXT

> FastText-inspired text embeddings with sentence-level encoding and configurable layers

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

Production-ready text embedding and classification system with three approaches: text-level (bag-of-words), word-level (subword features), and sentence-level (attention).

## ✨ Features

- **Three Approaches** - Text, word, and sentence-level classification
- **Configurable Layers** - Enable/disable features as needed
- **Sentence Encoding** - Optional self-attention for word order
- **Transfer Learning** - Pretrain + fine-tune workflow
- **Multiple Sizes** - From 4KB (compact) to full-featured models
- **Memory Efficient** - Sparse matrices, quantization, pruning
- **No Dependencies** - Pure C++17, stdlib only

## 🚀 Quick Start

```bash
# Build
make all

# Train
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 10

# Predict
echo "your text" | ./gladtotext-infer predict model.bin 1
```

## 📊 Three Approaches

| Approach | Speed | Size | Order | Typos | Use Case |
|----------|-------|------|-------|-------|----------|
| **Text-level** | ⚡⚡⚡ | 📦 | ❌ | ❌ | Simple keywords |
| **Word-level** | ⚡⚡ | 📦📦 | ❌ | ✅ | Typo-tolerant |
| **Sentence-level** | ⚡ | 📦📦📦 | ✅ | ✅ | Order matters |

### Text-Level (Fastest)
```bash
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 10
```

### Word-Level (Typo-Tolerant)
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 15 -minn 3 -maxn 6
```

### Sentence-Level (Order-Aware)
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -minn 3 -maxn 6 -sentence
```

## 📖 Documentation

- **[GUIDE.md](GUIDE.md)** - Complete guide with examples
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Development guide
- **[examples/](examples/)** - Working examples
- **[tests/](tests/)** - Test suite

## 🎯 Example

```bash
# Create training data
cat > train.txt << EOF
__greeting hello
__greeting hi there
__farewell goodbye
__farewell see you
EOF

# Train
./gladtotext supervised -input train.txt -output model \
  -dim 30 -epoch 20 -minCount 1

# Test
echo "hello friend" | ./gladtotext-infer predict model.bin 1
# Output: __greeting 0.485
```

## 🔧 Model Types

### Compact (4-20KB)
```bash
make compact
./gladtotext-compact train.txt model.bin 20 50 0.2
```

### Tiny (80-200KB) - Recommended
```bash
make tiny
./gladtotext-tiny train.txt model.bin -dim 30 -epoch 50 -lr 0.1
```

### Standard (400MB+)
```bash
./gladtotext supervised -input train.txt -output model -dim 100 -epoch 20
```

## 🧪 Testing

```bash
make test              # All tests
make test-unit         # Unit tests only
make test-integration  # Integration tests only
```

## 📈 Performance

- **Training**: 4K-10K examples/sec
- **Inference**: 0.1-2ms per prediction
- **Accuracy**: 85-95% (simple), 75-92% (complex)
- **Size**: 4KB (compact) to 400MB (standard)

## 🛠️ Requirements

- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Make
- No external dependencies

## 📝 Training Data Format

```
__label1 text for label 1
__label2 text for label 2
__label1 another example
```

## 🎓 Examples

```bash
make example-quickstart   # Quick demo
make example-intent       # Compare approaches
make example-transfer     # Transfer learning
make example-compact      # Compact models
make example-sentence     # Sentence encoding
```

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## 📄 License

MIT License - see LICENSE file

## 🙏 Acknowledgments

Inspired by FastText with enhancements:
- Sentence-level encoding
- Configurable layers
- Transfer learning
- Memory optimization

---

**Quick Links:** [Guide](GUIDE.md) | [Examples](examples/) | [Tests](tests/) | [Contributing](CONTRIBUTING.md)
