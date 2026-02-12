#!/bin/bash

# Quick test for sentence encoding feature

echo "=== Testing Sentence Encoding Feature ==="
echo ""

# Create minimal training data
cat > test_train.txt << 'EOF'
__pos good movie
__pos great film
__pos excellent show
__neg bad movie
__neg terrible film
__neg awful show
EOF

echo "1. Training WITHOUT sentence encoding..."
./gladtotext supervised -input test_train.txt -output test_bow -dim 20 -epoch 5 -lr 0.1 -minCount 1
echo ""

echo "2. Training WITH sentence encoding..."
./gladtotext supervised -input test_train.txt -output test_sent -dim 20 -epoch 5 -lr 0.1 -minCount 1 -sentence
echo ""

echo "3. Testing bag-of-words model:"
echo "good movie" | ./gladtotext-infer predict test_bow.bin 1
echo ""

echo "4. Testing sentence encoding model:"
echo "good movie" | ./gladtotext-infer predict test_sent.bin 1
echo ""

# Check model sizes
echo "5. Model sizes:"
ls -lh test_bow.bin test_sent.bin | awk '{print $9, $5}'
echo ""

echo "=== Test Complete ==="
