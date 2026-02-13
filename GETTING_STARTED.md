# Getting Started with GLADtoTEXT

Quick guide to get you up and running in 5 minutes.

## Prerequisites

- C++17 compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- Make
- Python 3.7+ (optional, for Python bindings)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/GLADtoTEXT.git
cd GLADtoTEXT
```

### 2. Build

```bash
make all
```

This creates:
- `gladtotext` - Training tool
- `gladtotext-infer` - Inference tool

### 3. Verify Installation

```bash
make test
```

All tests should pass ✅

## Your First Model

### Step 1: Create Training Data

```bash
cat > train.txt << EOF
__greeting hello
__greeting hi there
__greeting good morning
__farewell goodbye
__farewell see you later
__question what is this
__question how does it work
EOF
```

### Step 2: Train

```bash
./gladtotext supervised -input train.txt -output my_model \
  -dim 30 -epoch 20 -minCount 1
```

This creates `my_model.bin`

### Step 3: Test

```bash
echo "hello friend" | ./gladtotext-infer predict my_model.bin 1
```

Output:
```
__greeting 0.485
```

## Next Steps

### Try Different Approaches

**Text-level (fastest):**
```bash
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 10
```

**Word-level (typo-tolerant):**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 15 -minn 3 -maxn 6
```

**Sentence-level (order-aware):**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -minn 3 -maxn 6 -sentence
```

### Transfer Learning

**1. Pretrain on large corpus:**
```bash
./gladtotext cbow -input corpus.txt -output pretrained -dim 100 -epoch 10
```

**2. Fine-tune on labels:**
```bash
./gladtotext supervised -input train.txt -output model \
  -pretrained pretrained.bin -epoch 20
```

### Python Usage

**Install:**
```bash
pip install pybind11 numpy
pip install .
```

**Use:**
```python
import gladtotext

model = gladtotext.Model()
model.load_model("my_model.bin")

# Predict
predictions = model.predict("hello world", k=1)
print(predictions[0])  # ('__greeting', 0.485)

# Word vector
vec = model.get_word_vector("hello")
print(vec.shape)  # (30,)
```

## Examples

Run working examples:

```bash
make example-quickstart          # Quick demo
make example-intent              # Intent classification
make example-transfer            # Transfer learning
```

## Documentation

- **[README.md](README.md)** - Project overview
- **[GUIDE.md](GUIDE.md)** - Complete guide
- **[PYTHON_GUIDE.md](PYTHON_GUIDE.md)** - Python guide
- **[examples/](examples/)** - Working examples

## Common Issues

**Problem:** `gladtotext: command not found`
```bash
# Make sure you're in the project directory
cd GLADtoTEXT
./gladtotext --help
```

**Problem:** Build fails
```bash
# Check compiler version
g++ --version  # Should be 7.0 or higher

# Try clean build
make clean
make all
```

**Problem:** Python import fails
```bash
# Install dependencies
pip install pybind11 numpy

# Install package
pip install .
```

## What's Next?

1. **Read the guides** - [GUIDE.md](GUIDE.md) for detailed features
2. **Try examples** - See [examples/](examples/) directory
3. **Use Python** - See [PYTHON_GUIDE.md](PYTHON_GUIDE.md)
4. **Explore datasets** - Check dataset recommendations in guides
5. **Contribute** - See [CONTRIBUTING.md](CONTRIBUTING.md)

## Quick Reference

### Training Commands

```bash
# Supervised (classification)
./gladtotext supervised -input train.txt -output model -dim 100 -epoch 10

# Unsupervised (CBOW)
./gladtotext cbow -input corpus.txt -output model -dim 100 -epoch 10

# With pretrained embeddings
./gladtotext supervised -input train.txt -output model \
  -pretrained pretrained.bin -epoch 20

# With sentence encoding
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -sentence
```

### Inference Commands

```bash
# Predict
echo "your text" | ./gladtotext-infer predict model.bin 1

# Get word vectors
echo "hello" | ./gladtotext-infer print-word-vector model.bin
```

### Python API

```python
import gladtotext

# Load model
model = gladtotext.Model()
model.load_model("model.bin")

# Get info
info = model.get_info()

# Word vector
vec = model.get_word_vector("word")

# Predict
predictions = model.predict("text", k=3)
```

## Support

- **Issues:** Open an issue on GitHub
- **Documentation:** Check [docs/](docs/) directory
- **Examples:** See [examples/](examples/) directory

---

**Ready to build something awesome? Start with [GUIDE.md](GUIDE.md)!**
