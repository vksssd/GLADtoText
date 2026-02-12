# GLADtoTEXT Quick Reference

## Three Approaches to Intent Classification

### 🔤 Text-Level (Bag-of-Words)
```bash
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 10
```
- ⚡ Fastest
- 📦 Smallest
- ❌ No word order
- ❌ Poor OOV handling
- ✅ Use for: Simple keyword matching

### 📝 Word-Level (Subword Features)
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 30 -epoch 15 -minn 3 -maxn 6
```
- ⚡ Fast
- 📦 Medium size
- ❌ No word order
- ✅ Good OOV handling
- ✅ Handles typos
- ✅ Use for: Typo-tolerant classification

### 📖 Sentence-Level (Attention)
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -minn 3 -maxn 6 -sentence
```
- 🐢 Slower (1.5-2x)
- 📦 Larger
- ✅ Word order aware
- ✅ Good OOV handling
- ✅ Compositional meaning
- ✅ Use for: Complex intents, order matters

## Quick Comparison

| Feature | Text | Word | Sentence |
|---------|------|------|----------|
| Speed | ⚡⚡⚡ | ⚡⚡ | ⚡ |
| Size | 📦 | 📦📦 | 📦📦📦 |
| Order | ❌ | ❌ | ✅ |
| Typos | ❌ | ✅ | ✅ |
| Accuracy | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ |

## Common Commands

### Training
```bash
# CBOW (unsupervised)
./gladtotext cbow -input corpus.txt -output model -dim 100 -epoch 10

# Supervised (classification)
./gladtotext supervised -input labels.txt -output model -dim 50 -epoch 15

# With sentence encoding
./gladtotext supervised -input labels.txt -output model \
  -dim 50 -epoch 20 -sentence

# With pretrained embeddings
./gladtotext supervised -input labels.txt -output model \
  -pretrained pretrained.bin -epoch 10
```

### Inference
```bash
# Predict
echo "your text here" | ./gladtotext-infer predict model.bin 1

# Get word vectors
echo "word1 word2" | ./gladtotext-infer print-word-vector model.bin
```

### Testing
```bash
# Run all tests
make test

# Run specific test
./tests/t1  # Backbone test
./tests/t2  # Dictionary test
./tests/t3  # Unsupervised test
./tests/t4  # Supervised test
./tests/t5  # Memory test
```

## Parameter Guide

### Essential Parameters
- `-input FILE` - Training data file
- `-output NAME` - Output model name
- `-dim N` - Embedding dimension (20-100)
- `-epoch N` - Training epochs (5-30)

### Optional Parameters
- `-lr FLOAT` - Learning rate (0.01-0.2, default: 0.05)
- `-minCount N` - Min word frequency (default: 5)
- `-minn N` - Min char n-gram length (default: 3)
- `-maxn N` - Max char n-gram length (default: 6)
- `-bucket N` - Hash bucket size (default: 2000000)
- `-sentence` - Enable sentence encoding

### CBOW-Specific
- `-neg N` - Negative samples (default: 5)
- `-ws N` - Context window size (default: 5)

## Examples

### Example 1: Simple Intent Classifier
```bash
# Create training data
cat > intents.txt << EOF
__greeting hello
__greeting hi there
__farewell goodbye
__farewell see you
EOF

# Train
./gladtotext supervised -input intents.txt -output chatbot \
  -dim 30 -epoch 20 -minCount 1

# Test
echo "hello friend" | ./gladtotext-infer predict chatbot.bin 1
```

### Example 2: With Sentence Encoding
```bash
# Train with sentence encoding
./gladtotext supervised -input intents.txt -output chatbot \
  -dim 50 -epoch 20 -minCount 1 -sentence

# Test
echo "hello friend" | ./gladtotext-infer predict chatbot.bin 1
# Output: Model loaded: X words, dim=50, sentence-encoding=ON
```

### Example 3: Transfer Learning
```bash
# Step 1: Pretrain on large corpus
./gladtotext cbow -input wiki.txt -output pretrained \
  -dim 100 -epoch 10

# Step 2: Fine-tune on labeled data
./gladtotext supervised -input labels.txt -output classifier \
  -pretrained pretrained.bin -epoch 20
```

## Troubleshooting

### Problem: "Dictionary: 0 words"
**Solution:** Add `-minCount 1` if you have few examples

### Problem: Poor accuracy
**Solution:** 
- Increase epochs: `-epoch 30`
- Increase dimension: `-dim 100`
- Try sentence encoding: `-sentence`
- Add more training data

### Problem: Slow training
**Solution:**
- Reduce dimension: `-dim 30`
- Reduce epochs: `-epoch 10`
- Remove `-sentence` flag

### Problem: Large model size
**Solution:**
- Use compact models: `./gladtotext-compact`
- Use tiny models: `./gladtotext-tiny`
- Reduce dimension: `-dim 20`

## File Formats

### Training Data (Supervised)
```
__label1 text for label 1
__label2 text for label 2
__label1 another example for label 1
```

### Training Data (CBOW)
```
plain text corpus
one sentence per line
no labels needed
```

### Model Files
- `.bin` - Binary model file
- Contains: embeddings, attention, classifier, metadata

## Performance Tips

### For Speed
- Use text-level (no `-sentence`)
- Reduce dimension (`-dim 30`)
- Fewer epochs (`-epoch 10`)

### For Accuracy
- Use sentence-level (`-sentence`)
- Increase dimension (`-dim 100`)
- More epochs (`-epoch 30`)
- More training data

### For Size
- Use compact models
- Reduce dimension (`-dim 20`)
- Reduce bucket size (`-bucket 1000000`)

## See Also

- [INTENT_CLASSIFICATION_EXAMPLE.md](INTENT_CLASSIFICATION_EXAMPLE.md) - Complete example
- [SENTENCE_ENCODING.md](SENTENCE_ENCODING.md) - Sentence encoding guide
- [TRANSFER_LEARNING.md](TRANSFER_LEARNING.md) - Transfer learning guide
- [README.md](README.md) - Full documentation
