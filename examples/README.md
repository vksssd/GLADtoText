# GLADtoTEXT Examples

This directory contains example scripts demonstrating different features of GLADtoTEXT.

## Quick Start

```bash
# Run from project root
make example-quickstart
```

## Available Examples

### 1. quickstart.sh
**Quick demo of basic functionality**

```bash
cd examples
./quickstart.sh
```

Shows:
- Basic training
- Simple inference
- Model creation

Time: ~1 minute

### 2. example_intent_classification.sh
**Complete comparison of three approaches**

```bash
cd examples
./example_intent_classification.sh
```

Demonstrates:
- Text-level (bag-of-words)
- Word-level (subword features)
- Sentence-level (attention)
- Side-by-side comparison
- When to use each approach

Time: ~3 minutes

### 3. example_transfer_learning.sh
**Transfer learning workflow**

```bash
cd examples
./example_transfer_learning.sh
```

Shows:
- Pretraining on large corpus
- Fine-tuning on labeled data
- Performance comparison

Time: ~2 minutes

### 4. demo_compact.sh
**Ultra-compact models (4-20KB)**

```bash
cd examples
./demo_compact.sh
```

Demonstrates:
- Compact model training
- Size comparison
- Fast inference

Time: ~1 minute

### 5. demo_sentence_encoding.sh
**Sentence-level encoding**

```bash
cd examples
./demo_sentence_encoding.sh
```

Shows:
- Bag-of-words vs sentence encoding
- Word order awareness
- Attention mechanism

Time: ~2 minutes

## Running from Project Root

```bash
# Quick start
make example-quickstart

# Intent classification
make example-intent

# Transfer learning
make example-transfer

# Compact models
make example-compact

# Sentence encoding
make example-sentence
```

## Example Output

### Intent Classification Example

```
Training data: 32 examples, 5 intents
  - __book_flight (8 examples)
  - __cancel_flight (6 examples)
  - __check_status (6 examples)
  - __greeting (6 examples)
  - __farewell (6 examples)

Training text-level model...
Training word-level model...
Training sentence-level model...

Test Case: 'book a flight to paris'
  Text-level:     __book_flight 0.180
  Word-level:     __book_flight 0.180
  Sentence-level: __book_flight 0.144

Test Case: 'flight book to paris' (reversed)
  Text-level:     __book_flight -0.041 (confused)
  Word-level:     __book_flight -0.041 (confused)
  Sentence-level: __book_flight 0.144 (correct!)
```

## Creating Your Own Examples

1. Create a new script in `examples/`
2. Make it executable: `chmod +x examples/your_example.sh`
3. Add to Makefile (optional)
4. Document in this README

### Example Template

```bash
#!/bin/bash

echo "=== Your Example Name ==="
echo ""

# Create training data
cat > train.txt << 'EOF'
__label1 example text
__label2 another example
EOF

# Train model
../gladtotext supervised -input train.txt -output model \
  -dim 30 -epoch 10 -minCount 1

# Test
echo "test text" | ../gladtotext-infer predict model.bin 1

# Cleanup
rm -f train.txt model.bin

echo "=== Example complete ==="
```

## Tips

- Run examples from the `examples/` directory
- Or use `make example-*` from project root
- Check output for errors
- Models are cleaned up automatically
- Modify scripts to experiment

## Troubleshooting

### "Command not found"
Build the project first:
```bash
cd ..
make all
cd examples
```

### "Permission denied"
Make scripts executable:
```bash
chmod +x *.sh
```

### "File not found"
Run from examples directory or use make targets

## See Also

- [QUICK_REFERENCE.md](../QUICK_REFERENCE.md) - Command reference
- [INTENT_CLASSIFICATION_EXAMPLE.md](../INTENT_CLASSIFICATION_EXAMPLE.md) - Detailed guide
- [README.md](../README.md) - Main documentation
