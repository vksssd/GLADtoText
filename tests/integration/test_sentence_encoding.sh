#!/bin/bash

# Integration test: Sentence encoding

echo "=== Sentence Encoding Integration Test ==="
echo ""

# Create test data
cat > test_sent.txt << 'EOF'
__greeting hello friend
__greeting hi there
__farewell goodbye friend
__farewell see you later
EOF

echo "1. Training with sentence encoding..."
../../gladtotext supervised -input test_sent.txt -output test_sent_model \
  -dim 20 -epoch 15 -lr 0.1 -minCount 1 -sentence > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Training successful"
else
    echo "✗ Training failed"
    exit 1
fi

echo ""
echo "2. Testing inference..."
result=$(echo "hello friend" | ../../gladtotext-infer predict test_sent_model.bin 1 2>&1)

if [[ $result == *"sentence-encoding=ON"* ]]; then
    echo "✓ Sentence encoding detected in model"
else
    echo "✗ Sentence encoding not detected"
    exit 1
fi

if [[ $result == *"__greeting"* ]]; then
    echo "✓ Prediction correct"
else
    echo "✗ Prediction incorrect"
    exit 1
fi

# Cleanup
rm -f test_sent.txt test_sent_model.bin

echo ""
echo "=== Sentence encoding test passed! ==="
