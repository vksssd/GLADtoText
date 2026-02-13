# GLADtoTEXT Python Bindings

Python interface for GLADtoTEXT text embeddings and classification.

## Installation

### Prerequisites

```bash
# Install pybind11
pip install pybind11 numpy

# Or using conda
conda install -c conda-forge pybind11 numpy
```

### Build and Install

```bash
# From the project root directory
pip install .

# Or for development (editable install)
pip install -e .
```

## Quick Start

### 1. Train a Model (using C++ CLI)

```bash
# Create training data
cat > train.txt << EOF
__greeting hello
__greeting hi there
__farewell goodbye
__farewell see you
EOF

# Train model
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 20
```

### 2. Use from Python

```python
import gladtotext

# Load model
model = gladtotext.Model()
model.load_model("model.bin")

# Get model info
info = model.get_info()
print(f"Vocab: {info['vocab_size']}, Dim: {info['dim']}")

# Get word vectors
vec = model.get_word_vector("hello")
print(f"Vector shape: {vec.shape}")

# Predict (for supervised models)
predictions = model.predict("hello friend", k=2)
for label, score in predictions:
    print(f"{label}: {score:.4f}")
```

## API Reference

### Model Class

#### `Model()`
Create a new model instance.

#### `load_model(filename: str) -> bool`
Load a trained model from file.

**Parameters:**
- `filename`: Path to .bin model file

**Returns:**
- `True` if successful, `False` otherwise

**Example:**
```python
model = gladtotext.Model()
success = model.load_model("model.bin")
```

#### `get_word_vector(word: str) -> np.ndarray`
Get embedding vector for a word.

**Parameters:**
- `word`: Input word

**Returns:**
- NumPy array of shape `(dim,)` containing the word embedding

**Example:**
```python
vec = model.get_word_vector("hello")
print(vec.shape)  # (30,)
```

#### `predict(text: str, k: int = 1) -> List[Tuple[str, float]]`
Predict labels for text (supervised models only).

**Parameters:**
- `text`: Input text to classify
- `k`: Number of top predictions to return (default: 1)

**Returns:**
- List of tuples `(label, score)` sorted by score

**Example:**
```python
predictions = model.predict("hello world", k=3)
for label, score in predictions:
    print(f"{label}: {score:.4f}")
```

#### `get_info() -> dict`
Get model information.

**Returns:**
- Dictionary with keys:
  - `vocab_size`: Number of words in vocabulary
  - `dim`: Embedding dimension
  - `supervised`: Whether model is supervised
  - `sentence_encoder`: Whether sentence encoding is enabled
  - `num_labels`: Number of labels (supervised only)
  - `labels`: List of label names (supervised only)

**Example:**
```python
info = model.get_info()
print(f"Model has {info['vocab_size']} words")
```

## Complete Example

```python
import gladtotext
import numpy as np

# Load model
model = gladtotext.Model()
if not model.load_model("sentiment.bin"):
    raise RuntimeError("Failed to load model")

# Get info
info = model.get_info()
print(f"Loaded model with {info['num_labels']} labels")

# Batch prediction
texts = [
    "This is amazing!",
    "Terrible experience",
    "It's okay, nothing special"
]

for text in texts:
    predictions = model.predict(text, k=1)
    label, score = predictions[0]
    print(f"'{text}' -> {label} ({score:.3f})")

# Word similarity
words = ["good", "great", "excellent", "bad"]
vectors = [model.get_word_vector(w) for w in words]

# Compute cosine similarity
def cosine_similarity(v1, v2):
    return np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))

print("\nWord similarities:")
for i, w1 in enumerate(words):
    for j, w2 in enumerate(words[i+1:], i+1):
        sim = cosine_similarity(vectors[i], vectors[j])
        print(f"{w1} <-> {w2}: {sim:.3f}")
```

## Training from Python (Future)

Currently, training must be done using the C++ CLI. Python training API is planned for future releases.

For now, use:
```bash
# Unsupervised pretraining
./gladtotext cbow -input corpus.txt -output pretrained -dim 100 -epoch 10

# Supervised fine-tuning
./gladtotext supervised -input train.txt -output model \
  -pretrained pretrained.bin -epoch 20
```

## Performance Tips

1. **Batch Processing**: Process multiple texts in a loop for better throughput
2. **Model Loading**: Load model once and reuse for multiple predictions
3. **Vector Caching**: Cache word vectors if you need them multiple times

## Troubleshooting

### Import Error
```
ImportError: No module named 'gladtotext'
```
**Solution:** Make sure you've installed the package: `pip install .`

### Model Loading Fails
```python
model.load_model("model.bin")  # Returns False
```
**Solution:** 
- Check file path is correct
- Ensure model was trained with compatible version
- Verify file is not corrupted

### Prediction on Unsupervised Model
```
RuntimeError: Model is not supervised
```
**Solution:** Only supervised models (trained with `supervised` command) support prediction. Unsupervised models (CBOW) only provide word vectors.

## See Also

- [Main README](../README.md) - Project overview
- [GUIDE.md](../GUIDE.md) - Complete training guide
- [example_usage.py](example_usage.py) - Full Python example
