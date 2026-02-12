#!/bin/bash

set -e

echo "=== GLADtoTEXT Quick Start ==="
echo

# Build
echo "1. Building binaries..."
make clean && make all
echo "✓ Build complete"
echo

# Run tests
echo "2. Running tests..."
make test
echo "✓ All tests passed"
echo

# Example 1: CBOW
echo "3. Training CBOW model..."
cat > demo_corpus.txt << 'EOF'
the quick brown fox jumps over the lazy dog
the dog runs fast in the park
a cat sits on the mat
the fox is quick and clever
dogs and cats are pets
the lazy cat sleeps all day
quick brown animals run fast
EOF

./gladtotext cbow -input demo_corpus.txt -output demo_cbow -dim 50 -epoch 5 -minCount 1 -lr 0.05
echo "✓ CBOW model trained: demo_cbow.bin"
echo

# Get word vectors
echo "4. Getting word vectors..."
echo "dog cat fox" | ./gladtotext-infer print-word-vector demo_cbow.bin | head -3
echo "✓ Word vectors extracted"
echo

# Example 2: Supervised
echo "5. Training supervised classifier..."
cat > demo_sentiment.txt << 'EOF'
__label__positive this movie is great and amazing
__label__positive i love this film so much
__label__positive wonderful acting and great story
__label__positive best movie i have ever seen
__label__positive absolutely fantastic and brilliant
__label__negative this movie is terrible and boring
__label__negative i hate this film completely
__label__negative worst movie ever made
__label__negative awful acting and bad story
__label__negative completely disappointing and bad
EOF

./gladtotext supervised -input demo_sentiment.txt -output demo_classifier -dim 30 -epoch 10 -minCount 1 -lr 0.1
echo "✓ Classifier trained: demo_classifier.bin"
echo

# Predict
echo "6. Testing predictions..."
echo "Testing positive: 'this is a wonderful movie'"
echo "this is a wonderful movie" | ./gladtotext-infer predict demo_classifier.bin 2

echo
echo "Testing negative: 'terrible and boring film'"
echo "terrible and boring film" | ./gladtotext-infer predict demo_classifier.bin 2

echo
echo "=== Quick Start Complete! ==="
echo
echo "Next steps:"
echo "  - Train on your own data: ./gladtotext cbow -input yourdata.txt -output model"
echo "  - Read README.md for full documentation"
echo "  - Check tests/ directory for code examples"
