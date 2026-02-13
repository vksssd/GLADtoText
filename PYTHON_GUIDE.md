# GLADtoTEXT Python Guide

Complete guide for using GLADtoTEXT from Python.

## Table of Contents

1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [Training Workflow](#training-workflow)
4. [Python API](#python-api)
5. [Dataset Recommendations](#dataset-recommendations)
6. [Examples](#examples)
7. [Advanced Usage](#advanced-usage)
8. [Quick Reference](#quick-reference)

---

## Installation

### Prerequisites

```bash
# Install Python dependencies
pip install pybind11 numpy

# Or using requirements
pip install -r requirements.txt
```

### Build C++ Binaries (Required for Training)

```bash
# Build the training and inference tools
make all
```

### Install Python Package

```bash
# Option 1: Regular install
pip install .

# Option 2: Development install (editable)
pip install -e .

# Option 3: Using make
make python-install
```

### Verify Installation

```python
import gladtotext
print("GLADtoTEXT installed successfully!")
```

---

## Quick Start

### 1. Train a Model (CLI)

Currently, training is done via the C++ CLI:

```bash
# Create training data
cat > train.txt << EOF
__greeting hello
__greeting hi there
__greeting good morning
__farewell goodbye
__farewell see you later
__question what is this
__question how does it work
EOF

# Train supervised model
./gladtotext supervised -input train.txt -output chatbot \
  -dim 30 -epoch 20 -minCount 1
```

### 2. Use from Python

```python
import gladtotext

# Load model
model = gladtotext.Model()
model.load_model("chatbot.bin")

# Predict
predictions = model.predict("hello friend", k=1)
label, score = predictions[0]
print(f"Predicted: {label} (score: {score:.4f})")
```

---

## Training Workflow

### Two-Stage Training (Unsupervised + Supervised)

#### Stage 1: Unsupervised Pretraining

```bash
# Pretrain on large corpus
./gladtotext cbow -input wikipedia.txt -output pretrained \
  -dim 100 -epoch 10 -minn 3 -maxn 6
```

#### Stage 2: Supervised Fine-Tuning

```bash
# Fine-tune on labeled data
./gladtotext supervised -input labels.txt -output classifier \
  -pretrained pretrained.bin -epoch 20
```

#### Use from Python

```python
import gladtotext

# Load fine-tuned model
model = gladtotext.Model()
model.load_model("classifier.bin")

# Use for predictions
texts = [
    "book a flight to Paris",
    "what's the weather like",
    "play some music"
]

for text in texts:
    predictions = model.predict(text, k=1)
    print(f"{text} -> {predictions[0][0]}")
```

---

## Python API

### Model Class

#### Constructor

```python
model = gladtotext.Model()
```

Creates a new model instance. Must call `load_model()` before use.

#### load_model(filename)

```python
success = model.load_model("model.bin")
```

**Parameters:**
- `filename` (str): Path to trained model file

**Returns:**
- `bool`: True if successful, False otherwise

**Example:**
```python
model = gladtotext.Model()
if not model.load_model("model.bin"):
    print("Failed to load model")
    exit(1)
```

#### get_word_vector(word)

```python
vector = model.get_word_vector("hello")
```

**Parameters:**
- `word` (str): Input word

**Returns:**
- `numpy.ndarray`: Word embedding vector of shape `(dim,)`

**Example:**
```python
import numpy as np

vec = model.get_word_vector("hello")
print(f"Shape: {vec.shape}")
print(f"First 5 dims: {vec[:5]}")

# Compute similarity
vec1 = model.get_word_vector("good")
vec2 = model.get_word_vector("great")
similarity = np.dot(vec1, vec2) / (np.linalg.norm(vec1) * np.linalg.norm(vec2))
print(f"Similarity: {similarity:.3f}")
```

#### predict(text, k=1)

```python
predictions = model.predict("hello world", k=3)
```

**Parameters:**
- `text` (str): Input text to classify
- `k` (int, optional): Number of top predictions (default: 1)

**Returns:**
- `list`: List of tuples `(label, score)` sorted by score descending

**Raises:**
- `RuntimeError`: If model is not supervised

**Example:**
```python
# Single prediction
predictions = model.predict("hello friend")
label, score = predictions[0]
print(f"{label}: {score:.4f}")

# Top-k predictions
predictions = model.predict("hello friend", k=3)
for label, score in predictions:
    print(f"{label}: {score:.4f}")
```

#### get_info()

```python
info = model.get_info()
```

**Returns:**
- `dict`: Model information with keys:
  - `vocab_size` (int): Vocabulary size
  - `dim` (int): Embedding dimension
  - `supervised` (bool): Whether supervised
  - `sentence_encoder` (bool): Whether sentence encoding enabled
  - `num_labels` (int): Number of labels (supervised only)
  - `labels` (list): Label names (supervised only)

**Example:**
```python
info = model.get_info()
print(f"Vocabulary: {info['vocab_size']} words")
print(f"Dimension: {info['dim']}")
print(f"Model type: {'Supervised' if info['supervised'] else 'Unsupervised'}")

if info['supervised']:
    print(f"Labels: {info['labels']}")
```

---

## Dataset Recommendations

### For Unsupervised Pretraining (Large English Corpus)

**Recommended Options:**

1. **Wikipedia Dump** (Easiest to start)
   - Size: ~6GB compressed, ~20GB text
   - Clean, well-formatted English
   - Download: https://dumps.wikimedia.org/enwiki/

2. **Common Corpus** (Public Domain)
   - Size: 500 billion words
   - Multilingual, largest in English
   - Download: https://huggingface.co/PleIAs/common_corpus

3. **BookCorpus / Project Gutenberg**
   - Size: ~1GB (Gutenberg)
   - Public domain books
   - Download: https://www.gutenberg.org/

4. **C4 (Colossal Clean Crawled Corpus)**
   - Size: 750GB
   - Cleaned web text
   - Download: https://huggingface.co/datasets/c4

5. **FineWeb** (Latest, 2024)
   - Size: 15 trillion tokens
   - High-quality filtered web data
   - Download: https://huggingface.co/datasets/HuggingFaceFW/fineweb

### For Supervised Fine-Tuning

**Sentiment Analysis:**
- IMDb Reviews: 50,000 movie reviews
- Sentiment140: 1.6 million tweets
- Amazon Reviews: Millions of product reviews

**Intent Classification:**
- ATIS: Airline travel information
- SNIPS: 7 intents (weather, music, etc.)
- Banking77: 77 banking intents

**Topic Classification:**
- 20 Newsgroups: 20,000 posts across 20 topics
- AG News: 120,000 news articles, 4 categories
- DBpedia: 560,000 Wikipedia articles, 14 classes

**Quick Start Recommendation:**
```bash
# Start with Wikipedia (manageable) + AG News (easy format)
# 1. Pretrain on Wikipedia
./gladtotext cbow -input wikipedia.txt -output pretrained -dim 100 -epoch 10

# 2. Fine-tune on AG News
./gladtotext supervised -input ag_news_train.txt -output classifier \
  -pretrained pretrained.bin -epoch 20
```

---

## Examples

### Example 1: Sentiment Analysis

```python
import gladtotext

# Load sentiment model
model = gladtotext.Model()
model.load_model("sentiment.bin")

# Analyze sentiment
reviews = [
    "This product is amazing! Highly recommend.",
    "Terrible quality, waste of money.",
    "It's okay, nothing special."
]

for review in reviews:
    predictions = model.predict(review, k=1)
    sentiment, score = predictions[0]
    print(f"Review: {review}")
    print(f"Sentiment: {sentiment} (confidence: {score:.2f})\n")
```

### Example 2: Intent Classification

```python
import gladtotext

# Load intent classifier
model = gladtotext.Model()
model.load_model("intents.bin")

# Classify user intents
user_inputs = [
    "book a flight to New York",
    "what's the weather tomorrow",
    "play some jazz music",
    "set an alarm for 7am"
]

for text in user_inputs:
    predictions = model.predict(text, k=2)
    print(f"Input: {text}")
    print("Top intents:")
    for intent, score in predictions:
        print(f"  {intent}: {score:.3f}")
    print()
```

### Example 3: Word Similarity

```python
import gladtotext
import numpy as np

# Load model
model = gladtotext.Model()
model.load_model("model.bin")

def cosine_similarity(v1, v2):
    return np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))

# Find similar words
target_word = "king"
candidates = ["queen", "prince", "man", "woman", "royal", "castle"]

target_vec = model.get_word_vector(target_word)

similarities = []
for word in candidates:
    vec = model.get_word_vector(word)
    sim = cosine_similarity(target_vec, vec)
    similarities.append((word, sim))

# Sort by similarity
similarities.sort(key=lambda x: x[1], reverse=True)

print(f"Words similar to '{target_word}':")
for word, sim in similarities:
    print(f"  {word}: {sim:.3f}")
```

### Example 4: Batch Processing

```python
import gladtotext
import time

# Load model
model = gladtotext.Model()
model.load_model("classifier.bin")

# Batch prediction
texts = [f"sample text {i}" for i in range(1000)]

start = time.time()
results = []
for text in texts:
    predictions = model.predict(text, k=1)
    results.append(predictions[0])
elapsed = time.time() - start

print(f"Processed {len(texts)} texts in {elapsed:.2f}s")
print(f"Throughput: {len(texts)/elapsed:.0f} texts/sec")
```

### Example 5: Text Embeddings for Clustering

```python
import gladtotext
import numpy as np
from sklearn.cluster import KMeans

# Load model
model = gladtotext.Model()
model.load_model("model.bin")

# Get sentence embeddings
sentences = [
    "I love this product",
    "Great quality and fast shipping",
    "Terrible experience, very disappointed",
    "Worst purchase ever",
    "Amazing! Exceeded expectations",
    "Poor quality, not worth the price"
]

# Get embeddings (average word vectors)
embeddings = []
for sentence in sentences:
    words = sentence.lower().split()
    word_vecs = [model.get_word_vector(w) for w in words]
    sentence_vec = np.mean(word_vecs, axis=0)
    embeddings.append(sentence_vec)

embeddings = np.array(embeddings)

# Cluster
kmeans = KMeans(n_clusters=2, random_state=42)
labels = kmeans.fit_predict(embeddings)

# Print clusters
for i in range(2):
    print(f"\nCluster {i}:")
    for j, label in enumerate(labels):
        if label == i:
            print(f"  - {sentences[j]}")
```

---

## Advanced Usage

### Custom Preprocessing

```python
import gladtotext
import re

class TextClassifier:
    def __init__(self, model_path):
        self.model = gladtotext.Model()
        if not self.model.load_model(model_path):
            raise RuntimeError(f"Failed to load model: {model_path}")
    
    def preprocess(self, text):
        # Lowercase
        text = text.lower()
        # Remove special characters
        text = re.sub(r'[^a-z0-9\s]', '', text)
        # Remove extra whitespace
        text = ' '.join(text.split())
        return text
    
    def predict(self, text, k=1):
        text = self.preprocess(text)
        return self.model.predict(text, k)

# Usage
classifier = TextClassifier("model.bin")
predictions = classifier.predict("Hello!!! How are you???", k=2)
```

### Model Ensemble

```python
import gladtotext
import numpy as np

class EnsembleClassifier:
    def __init__(self, model_paths):
        self.models = []
        for path in model_paths:
            model = gladtotext.Model()
            if model.load_model(path):
                self.models.append(model)
    
    def predict(self, text, k=1):
        # Get predictions from all models
        all_predictions = {}
        
        for model in self.models:
            predictions = model.predict(text, k=5)
            for label, score in predictions:
                if label not in all_predictions:
                    all_predictions[label] = []
                all_predictions[label].append(score)
        
        # Average scores
        averaged = []
        for label, scores in all_predictions.items():
            avg_score = np.mean(scores)
            averaged.append((label, avg_score))
        
        # Sort and return top-k
        averaged.sort(key=lambda x: x[1], reverse=True)
        return averaged[:k]

# Usage
ensemble = EnsembleClassifier([
    "model1.bin",
    "model2.bin",
    "model3.bin"
])
predictions = ensemble.predict("test text", k=1)
```

### Caching for Performance

```python
import gladtotext
from functools import lru_cache

class CachedModel:
    def __init__(self, model_path):
        self.model = gladtotext.Model()
        self.model.load_model(model_path)
    
    @lru_cache(maxsize=10000)
    def get_word_vector(self, word):
        vec = self.model.get_word_vector(word)
        return tuple(vec)  # Convert to tuple for caching
    
    @lru_cache(maxsize=1000)
    def predict(self, text, k=1):
        predictions = self.model.predict(text, k)
        return tuple(predictions)  # Convert to tuple for caching

# Usage
model = CachedModel("model.bin")

# First call - computed
vec1 = model.get_word_vector("hello")

# Second call - cached (faster)
vec2 = model.get_word_vector("hello")
```

---

## Performance Tips

1. **Load Once**: Load model once and reuse for multiple predictions
2. **Batch Processing**: Process texts in batches for better throughput
3. **Cache Vectors**: Cache frequently used word vectors
4. **Preprocessing**: Minimize preprocessing overhead
5. **Model Size**: Use compact/tiny models for faster inference

## Troubleshooting

### Build Errors

**Error:** `pybind11/pybind11.h: No such file or directory`

**Solution:**
```bash
pip install pybind11
```

**Error:** `fatal error: Python.h: No such file or directory`

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install python3-dev

# macOS
brew install python3

# Or use conda
conda install python-dev
```

### Runtime Errors

**Error:** `ImportError: No module named 'gladtotext'`

**Solution:**
```bash
pip install .
# Or
pip install -e .
```

**Error:** `RuntimeError: Model is not supervised`

**Solution:** Only supervised models support `predict()`. Train with:
```bash
./gladtotext supervised -input train.txt -output model
```

---

## Quick Reference

### Installation
```bash
pip install pybind11 numpy
make all
pip install .
```

### Basic Usage
```python
import gladtotext

# Load model
model = gladtotext.Model()
model.load_model("model.bin")

# Get info
info = model.get_info()

# Word vector
vec = model.get_word_vector("hello")

# Predict
predictions = model.predict("text", k=1)
label, score = predictions[0]
```

### Training Workflow
```bash
# 1. Pretrain (unsupervised)
./gladtotext cbow -input corpus.txt -output pretrained -dim 100 -epoch 10

# 2. Fine-tune (supervised)
./gladtotext supervised -input labels.txt -output model \
  -pretrained pretrained.bin -epoch 20
```

### API Methods

| Method | Description | Returns |
|--------|-------------|---------|
| `load_model(path)` | Load trained model | `bool` |
| `get_word_vector(word)` | Get word embedding | `np.ndarray` |
| `predict(text, k=1)` | Classify text | `[(label, score), ...]` |
| `get_info()` | Model metadata | `dict` |

### Common Patterns

**Batch Processing:**
```python
texts = ["text1", "text2", "text3"]
results = [model.predict(t, k=1) for t in texts]
```

**Word Similarity:**
```python
import numpy as np
v1 = model.get_word_vector("king")
v2 = model.get_word_vector("queen")
sim = np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
```

**Top-K Predictions:**
```python
predictions = model.predict("text", k=3)
for label, score in predictions:
    print(f"{label}: {score:.3f}")
```

### Troubleshooting

| Problem | Solution |
|---------|----------|
| Import error | `pip install .` |
| Build error | `pip install pybind11` |
| Model not supervised | Train with `supervised` command |
| Python.h missing | Install python3-dev |

---

## See Also

- [README.md](README.md) - Project overview
- [GUIDE.md](GUIDE.md) - Complete training guide
- [python/README.md](python/README.md) - Python API reference
- [python/example_usage.py](python/example_usage.py) - Example code
