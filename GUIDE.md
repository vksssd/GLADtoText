# GLADtoTEXT Complete Guide

## Table of Contents

1. [Getting Started](#getting-started)
2. [Quick Reference](#quick-reference)
3. [Three Approaches](#three-approaches)
4. [Model Types](#model-types)
5. [Advanced Features](#advanced-features)
6. [Architecture](#architecture)
7. [Examples](#examples)

---

## Getting Started

### Installation

```bash
# Clone repository
git clone https://github.com/yourusername/GLADtoTEXT.git
cd GLADtoTEXT

# Build
make all

# Run tests
make test
```

### Your First Model

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
```

---

## Quick Reference

### Common Commands

```bash
# Text-level (fastest)
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 10

# Word-level (typo-tolerant)
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 15 -minn 3 -maxn 6

# Sentence-level (order-aware)
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -minn 3 -maxn 6 -sentence

# Transfer learning
./gladtotext cbow -input corpus.txt -output pretrained -dim 100 -epoch 10
./gladtotext supervised -input train.txt -output model \
  -pretrained pretrained.bin -epoch 20

# Inference
echo "your text" | ./gladtotext-infer predict model.bin 1
```

### Parameters

| Parameter | Description | Default | Range |
|-----------|-------------|---------|-------|
| `-input` | Training data file | Required | - |
| `-output` | Output model name | Required | - |
| `-dim` | Embedding dimension | 100 | 20-200 |
| `-epoch` | Training epochs | 10 | 5-50 |
| `-lr` | Learning rate | 0.05 | 0.01-0.2 |
| `-minCount` | Min word frequency | 5 | 1-100 |
| `-minn` | Min char n-gram | 3 | 1-6 |
| `-maxn` | Max char n-gram | 6 | 3-10 |
| `-sentence` | Enable sentence encoding | false | - |
| `-pretrained` | Pretrained model | - | - |

---

## Three Approaches

### 1. Text-Level (Bag-of-Words)

**How it works:**
- Treats text as unordered set of words
- No subword features
- Fastest approach

**When to use:**
- Simple keyword matching
- Speed is critical
- Limited training data

**Example:**
```bash
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 10
```

**Pros:**
- ⚡ Fastest (10K examples/sec)
- 📦 Smallest models
- 🎯 Simple and effective

**Cons:**
- ❌ No word order
- ❌ Poor OOV handling
- ❌ No typo tolerance

### 2. Word-Level (Subword Features)

**How it works:**
- Each word encoded with char n-grams
- Grammar units
- Phonetic encoding
- Still no word order

**When to use:**
- Need typo tolerance
- OOV words common
- User input has errors

**Example:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 15 -minn 3 -maxn 6
```

**Pros:**
- ⚡ Fast (7K examples/sec)
- ✅ Handles typos
- ✅ Good OOV handling

**Cons:**
- ❌ No word order
- 📦 Larger models

### 3. Sentence-Level (Attention)

**How it works:**
- Position encoding
- Self-attention (Q/K/V)
- Word order awareness
- All subword features

**When to use:**
- Word order affects meaning
- Complex compositional intents
- Negation and modifiers

**Example:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -minn 3 -maxn 6 -sentence
```

**Pros:**
- ✅ Word order aware
- ✅ Best accuracy
- ✅ Compositional meaning

**Cons:**
- 🐢 Slower (4K examples/sec)
- 📦 Larger models

### Comparison

| Feature | Text | Word | Sentence |
|---------|------|------|----------|
| Speed | ⚡⚡⚡ | ⚡⚡ | ⚡ |
| Size | 📦 | 📦📦 | 📦📦📦 |
| Order | ❌ | ❌ | ✅ |
| Typos | ❌ | ✅ | ✅ |
| OOV | ❌ | ✅ | ✅ |
| Accuracy | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ |

---

## Model Types

### Compact Models (4-20KB)

**Features:**
- Word-level only
- No char n-grams
- Ultra-small size

**Usage:**
```bash
make compact
./gladtotext-compact train.txt model.bin 20 50 0.2
```

**Best for:**
- Mobile apps
- Edge devices
- Memory-constrained environments

### Tiny Models (80-200KB)

**Features:**
- ALL features included
- Sparse storage
- 93x smaller than standard

**Usage:**
```bash
make tiny
./gladtotext-tiny train.txt model.bin -dim 30 -epoch 50 -lr 0.1
```

**Best for:**
- Production deployment
- Balance of size and features
- Recommended for most use cases

### Standard Models (400MB+)

**Features:**
- ALL features
- Dense storage
- Best accuracy

**Usage:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 100 -epoch 20
```

**Best for:**
- Research
- Maximum accuracy
- When size doesn't matter

---

## Advanced Features

### Transfer Learning

**Workflow:**
1. Pretrain on large unlabeled corpus
2. Fine-tune on small labeled dataset

**Example:**
```bash
# Step 1: Pretrain
./gladtotext cbow -input wiki.txt -output pretrained \
  -dim 100 -epoch 10

# Step 2: Fine-tune
./gladtotext supervised -input labels.txt -output classifier \
  -pretrained pretrained.bin -epoch 20
```

**Benefits:**
- Better accuracy with less labeled data
- Faster convergence
- Better generalization

### Sentence Encoding

**Features:**
- Position encoding (Transformer-style)
- Self-attention mechanism
- Word order awareness

**Example:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -sentence
```

**When to use:**
- "not good" vs "good" (negation)
- "book flight" vs "flight book" (order)
- "how are you" (compositional meaning)

### Configurable Layers

**Available layers:**
1. Word embeddings
2. Character n-grams
3. Grammar units
4. Phonetic encoding
5. Vector attention
6. Sentence encoder

**Control via config:**
```cpp
ModelConfig cfg;
cfg.use_char_ngrams = true;
cfg.use_grammar_units = false;
cfg.use_sentence_encoder = true;
cfg.saveToFile("model.config");
```

---

## Architecture

### Core Components

```
Input Text
    ↓
Dictionary (word/grammar/phonetic vocab)
    ↓
Subword Features
├── Character n-grams (FNV-1a hashing)
├── Grammar units (frequent substrings)
└── Phonetic encoding (sound-based)
    ↓
Embeddings (word + subword)
    ↓
Vector Attention (feature weighting)
    ↓
[Optional] Sentence Encoder
├── Position encoding
└── Self-attention (Q/K/V)
    ↓
Backbone (combined representation)
    ↓
Classification Head
    ↓
Output (label + score)
```

### Key Algorithms

**1. Character N-grams (FNV-1a)**
```
word = "hello"
w = "<hello>"
Extract: <he, hel, hell, hello, he, hel, ...
Hash: FNV-1a(ngram) % bucket_size
```

**2. Vector Attention**
```
for each dimension d:
  score = w[d] * embedding[d]
  attention[d] = sigmoid(score)
  output[d] = attention[d] * embedding[d]
```

**3. Self-Attention**
```
Q = W_q × word_embeddings
K = W_k × word_embeddings
V = W_v × word_embeddings

scores = softmax(Q × K^T / sqrt(dim))
output = scores × V
```

### Memory Optimization

**Sparse Matrices:**
- Only store non-zero values
- 90% memory reduction
- Automatic pruning

**Quantization:**
- int8 instead of float32
- 4x size reduction
- Minimal accuracy loss

**Lazy Initialization:**
- Initialize on first access
- Save memory for unused features

---

## Examples

### Example 1: Simple Chatbot

```bash
# Create training data
cat > intents.txt << EOF
__greeting hello
__greeting hi there
__greeting good morning
__farewell goodbye
__farewell see you later
__question what is this
__question how does it work
EOF

# Train
./gladtotext supervised -input intents.txt -output chatbot \
  -dim 30 -epoch 20 -minCount 1

# Test
echo "hello friend" | ./gladtotext-infer predict chatbot.bin 1
# Output: __greeting 0.485
```

### Example 2: Sentiment Analysis

```bash
# Create training data
cat > sentiment.txt << EOF
__positive great movie
__positive excellent film
__positive amazing show
__negative bad movie
__negative terrible film
__negative awful show
EOF

# Train with sentence encoding (for negation)
./gladtotext supervised -input sentiment.txt -output sentiment \
  -dim 50 -epoch 20 -minCount 1 -sentence

# Test
echo "not good" | ./gladtotext-infer predict sentiment.bin 1
# Correctly handles negation
```

### Example 3: Intent Classification with Transfer Learning

```bash
# Pretrain on large corpus
./gladtotext cbow -input wiki.txt -output pretrained \
  -dim 100 -epoch 10

# Fine-tune on intents
./gladtotext supervised -input intents.txt -output classifier \
  -pretrained pretrained.bin -epoch 20 -minCount 1

# Test
echo "book a flight" | ./gladtotext-infer predict classifier.bin 1
```

### Example 4: Compare All Approaches

```bash
# Run complete comparison
cd examples
./example_intent_classification.sh
```

This will train and test all three approaches side-by-side.

---

## Troubleshooting

### Problem: Poor accuracy

**Solutions:**
- Increase epochs: `-epoch 30`
- Increase dimension: `-dim 100`
- Try sentence encoding: `-sentence`
- Use transfer learning
- Add more training data

### Problem: Slow training

**Solutions:**
- Reduce dimension: `-dim 30`
- Reduce epochs: `-epoch 10`
- Remove `-sentence` flag
- Use text-level approach

### Problem: Large model size

**Solutions:**
- Use compact models: `make compact`
- Use tiny models: `make tiny`
- Reduce dimension: `-dim 20`

### Problem: "Dictionary: 0 words"

**Solution:**
Add `-minCount 1` if you have few examples

### Problem: Out of memory

**Solutions:**
- Reduce dimension
- Reduce bucket size: `-bucket 1000000`
- Use sparse matrices (tiny models)

---

## Performance Tips

### For Speed
- Use text-level (no `-sentence`)
- Reduce dimension (`-dim 30`)
- Fewer epochs (`-epoch 10`)

### For Accuracy
- Use sentence-level (`-sentence`)
- Increase dimension (`-dim 100`)
- More epochs (`-epoch 30`)
- Use transfer learning

### For Size
- Use compact models
- Reduce dimension (`-dim 20`)
- Reduce bucket size

---

## Training Data Format

### Supervised (Classification)
```
__label1 text for label 1
__label2 text for label 2
__label1 another example
```

### CBOW (Unsupervised)
```
plain text corpus
one sentence per line
no labels needed
```

---

## See Also

- **README.md** - Project overview
- **examples/** - Working examples
- **tests/** - Test suite
- **CONTRIBUTING.md** - Development guide
