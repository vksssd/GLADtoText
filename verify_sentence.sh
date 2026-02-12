#!/bin/bash

# Verify sentence encoding works correctly

echo "=== Verifying Sentence Encoding ==="
echo ""

# Create training data with more examples
cat > verify_train.txt << 'EOF'
__greeting hello
__greeting hi
__greeting hey
__greeting good morning
__greeting hello there
__greeting hi friend
__farewell goodbye
__farewell bye
__farewell see you
__farewell take care
__farewell goodbye friend
__farewell see you later
EOF

echo "Training data: 12 examples, 2 classes"
echo ""

# Train with sentence encoding
echo "Training with sentence encoding..."
./gladtotext supervised -input verify_train.txt -output verify_model \
  -dim 30 -epoch 20 -lr 0.1 -minCount 1 -sentence
echo ""

# Test predictions
echo "Testing predictions:"
echo ""

echo "Test 1: 'hello' (should be __greeting)"
echo "hello" | ./gladtotext-infer predict verify_model.bin 1
echo ""

echo "Test 2: 'goodbye' (should be __farewell)"
echo "goodbye" | ./gladtotext-infer predict verify_model.bin 1
echo ""

echo "Test 3: 'hi friend' (should be __greeting)"
echo "hi friend" | ./gladtotext-infer predict verify_model.bin 1
echo ""

echo "Test 4: 'see you later' (should be __farewell)"
echo "see you later" | ./gladtotext-infer predict verify_model.bin 1
echo ""

echo "=== Verification Complete ==="
