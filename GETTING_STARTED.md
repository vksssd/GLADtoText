# Getting Started with GLADtoTEXT

## Installation

### Build from Source

```bash
# Clone or download the repository
cd GLADtoTEXT

# Build binaries
make all

# Run tests to verify installation
make test
```

You should see:
```
All tests passed!
```

## Your First Model

### Option 1: Quick Demo

Run the quickstart script to see everything in action:

```bash
./quickstart.sh
```

This will:
1. Build the binaries
2. Run all tests
3. Train a CBOW model
4. Train a classifier
5. Show predictions

### Option 2: Step-by-Step

#### 1. Train Word Embeddings

Create a text file with one sentence per line:

```bash
cat > my_corpus.txt << EOF
the quick brown fox jumps over the lazy dog
dogs and cats are wonderful pets
animals need love and care
EOF
```

Train embeddings:

```bash
./gladtotext cbow \
  -input my_corpus.txt \
  -output my_model \
  -dim 100 \
  -epoch 10 \
  -minCount 1
```

Get word vectors:

```bash
echo "dog cat fox" | ./gladtotext-infer print-word-vector my_model.bin
```

#### 2. Train a Classifier

Create labeled data:

```bash
cat > my_labels.txt << EOF
__label__positive this is great
__label__positive wonderful experience
__label__negative this is bad
__label__negative terrible service
EOF
```

Train classifier:

```bash
./gladtotext supervised \
  -input my_labels.txt \
  -output my_classifier \
  -dim 50 \
  -epoch 20 \
  -minCount 1
```

Make predictions:

```bash
echo "wonderful service" | ./gladtotext-infer predict my_classifier.bin 1
# Output: __label__positive 1.23
```

## Transfer Learning (Recommended)

For best results, use transfer learning:

### Step 1: Pretrain on Large Corpus

```bash
# Use a large unlabeled corpus
./gladtotext cbow \
  -input large_corpus.txt \
  -output pretrained \
  -dim 100 \
  -epoch 10 \
  -minCount 5
```

### Step 2: Fine-tune on Labeled Data

```bash
# Use your labeled data
./gladtotext supervised \
  -input my_labels.txt \
  -output my_classifier \
  -pretrained pretrained.bin \
  -epoch 20 \
  -lr 0.1
```

### Step 3: Use the Model

```bash
echo "your text here" | ./gladtotext-infer predict my_classifier.bin 1
```

**Benefits:**
- 5-10% better accuracy
- Faster convergence
- Works with less labeled data

See [TRANSFER_LEARNING.md](TRANSFER_LEARNING.md) for complete guide.

## Common Use Cases

### Sentiment Analysis

```bash
# Pretrain on product reviews
./gladtotext cbow -input reviews.txt -output review_embeddings -dim 100

# Fine-tune for sentiment
./gladtotext supervised -input sentiment_labels.txt -output sentiment_clf \
  -pretrained review_embeddings.bin -epoch 20

# Classify new reviews
echo "great product fast shipping" | ./gladtotext-infer predict sentiment_clf.bin 1
```

### Topic Classification

```bash
# Pretrain on news articles
./gladtotext cbow -input news.txt -output news_embeddings -dim 200

# Fine-tune for topics
./gladtotext supervised -input topic_labels.txt -output topic_clf \
  -pretrained news_embeddings.bin -epoch 15

# Classify articles
echo "stock market rises today" | ./gladtotext-infer predict topic_clf.bin 3
```

### Intent Detection

```bash
# Pretrain on conversations
./gladtotext cbow -input conversations.txt -output conv_embeddings -dim 100

# Fine-tune for intents
./gladtotext supervised -input intent_labels.txt -output intent_clf \
  -pretrained conv_embeddings.bin -epoch 25

# Detect intent
echo "book a flight to paris" | ./gladtotext-infer predict intent_clf.bin 1
```

## Parameters Guide

### CBOW Training

| Parameter | Description | Default | Recommended |
|-----------|-------------|---------|-------------|
| `-input` | Training file | required | - |
| `-output` | Model name | required | - |
| `-dim` | Embedding dimension | 100 | 50-300 |
| `-epoch` | Training epochs | 5 | 10-20 |
| `-lr` | Learning rate | 0.05 | 0.025-0.1 |
| `-minCount` | Min word frequency | 5 | 1-10 |
| `-ws` | Window size | 5 | 3-7 |
| `-neg` | Negative samples | 5 | 3-10 |
| `-minn` | Min n-gram length | 3 | 3-4 |
| `-maxn` | Max n-gram length | 6 | 5-6 |
| `-bucket` | Hash bucket size | 2000000 | 1M-5M |

### Supervised Training

| Parameter | Description | Default | Recommended |
|-----------|-------------|---------|-------------|
| `-input` | Labeled file | required | - |
| `-output` | Model name | required | - |
| `-pretrained` | Pretrained model | none | recommended |
| `-dim` | Embedding dimension | 100 | 50-200 |
| `-epoch` | Training epochs | 5 | 10-50 |
| `-lr` | Learning rate | 0.05 | 0.05-0.2 |
| `-minCount` | Min word frequency | 5 | 1-5 |

**Tips:**
- Use higher `-lr` (0.1-0.2) when fine-tuning with `-pretrained`
- Use more `-epoch` (20-50) with small labeled datasets
- Larger `-dim` (200-300) for complex tasks

## Troubleshooting

### Issue: "Cannot open model file"

**Solution:** Check file path and ensure model was trained successfully.

```bash
ls -lh *.bin  # Check if model file exists
```

### Issue: Poor accuracy

**Solutions:**
1. Use transfer learning (pretrain + fine-tune)
2. Increase training epochs
3. Add more labeled data
4. Try different learning rates

```bash
# Try transfer learning
./gladtotext cbow -input large_corpus.txt -output pretrained -dim 100
./gladtotext supervised -input labels.txt -output classifier \
  -pretrained pretrained.bin -epoch 30 -lr 0.1
```

### Issue: Slow training

**Solutions:**
1. Reduce dimension: `-dim 50`
2. Reduce window: `-ws 3`
3. Reduce negatives: `-neg 3`
4. Use smaller corpus for testing

### Issue: Out of memory

**Solutions:**
1. Reduce vocabulary: increase `-minCount`
2. Reduce dimension: `-dim 50`
3. Reduce bucket size: `-bucket 1000000`

## Next Steps

1. **Read the documentation:**
   - [README.md](README.md) - Complete user guide
   - [ARCHITECTURE.md](ARCHITECTURE.md) - Technical details
   - [TRANSFER_LEARNING.md](TRANSFER_LEARNING.md) - Transfer learning guide

2. **Try the examples:**
   - `./quickstart.sh` - Full demo
   - `./test_transfer_learning.sh` - Transfer learning demo

3. **Train on your data:**
   - Prepare your corpus (one sentence per line)
   - Pretrain embeddings on large corpus
   - Fine-tune classifier on labeled data

4. **Deploy to production:**
   - Use `./gladtotext-infer` for inference
   - Batch process with pipes
   - Integrate into your application

## Support

- Check [README.md](README.md) for detailed usage
- See [ARCHITECTURE.md](ARCHITECTURE.md) for technical details
- Read [TRANSFER_LEARNING.md](TRANSFER_LEARNING.md) for best practices
- Review example scripts in the repository

## Summary

GLADtoTEXT provides:
- ✅ FastText-compatible embeddings
- ✅ Text classification
- ✅ Transfer learning
- ✅ Production-ready tools

**Quick commands:**
```bash
# Build and test
make all && make test

# Train with transfer learning
./gladtotext cbow -input corpus.txt -output pretrained -dim 100
./gladtotext supervised -input labels.txt -output classifier \
  -pretrained pretrained.bin -epoch 20

# Predict
echo "text" | ./gladtotext-infer predict classifier.bin 1
```

Happy training! 🚀
