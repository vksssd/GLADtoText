#!/bin/bash

# Integration test: Transfer learning

echo "=== Transfer Learning Integration Test ==="
echo ""

# Create corpus for pretraining
cat > corpus.txt << 'EOF'
the quick brown fox jumps over the lazy dog
a journey of a thousand miles begins with a single step
to be or not to be that is the question
all that glitters is not gold
EOF

echo "1. Pretraining with CBOW..."
../../gladtotext cbow -input corpus.txt -output pretrained \
  -dim 30 -epoch 5 -minCount 1 > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Pretraining successful"
else
    echo "✗ Pretraining failed"
    exit 1
fi

# Create labeled data
cat > labels.txt << 'EOF'
__positive good journey
__positive great step
__negative bad question
__negative not good
EOF

echo ""
echo "2. Fine-tuning with pretrained embeddings..."
../../gladtotext supervised -input labels.txt -output finetuned \
  -pretrained pretrained.bin -epoch 10 -minCount 1 > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Fine-tuning successful"
else
    echo "✗ Fine-tuning failed"
    exit 1
fi

echo ""
echo "3. Testing inference..."
result=$(echo "good step" | ../../gladtotext-infer predict finetuned.bin 1 2>/dev/null | head -1)

if [[ $result == *"__positive"* ]]; then
    echo "✓ Prediction correct: $result"
else
    echo "✗ Prediction incorrect: $result"
    exit 1
fi

# Cleanup
rm -f corpus.txt labels.txt pretrained.bin finetuned.bin

echo ""
echo "=== Transfer learning test passed! ==="
