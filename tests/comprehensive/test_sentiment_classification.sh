#!/bin/bash

# Comprehensive test: Sentiment classification (3 classes)

echo "=== Sentiment Classification Test ==="
echo ""

TRAIN_DATA="../../tests/data/sentiment_large.txt"
VALID_DATA="../../tests/data/sentiment_validation.txt"
MODEL_NAME="sentiment_model"

# Test 1: Basic training
echo "1. Training sentiment classifier..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output $MODEL_NAME \
  -dim 100 \
  -epoch 30 \
  -lr 0.1 \
  -minCount 1 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Training successful"
else
    echo "✗ Training failed"
    exit 1
fi

# Test 2: Validation
echo ""
echo "2. Training with validation..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -valid $VALID_DATA \
  -output ${MODEL_NAME}_val \
  -dim 100 \
  -epoch 30 \
  -lr 0.1 \
  -minCount 1 \
  -early-stop \
  -patience 5 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Validation training successful"
else
    echo "✗ Validation training failed"
    exit 1
fi

# Test 3: Batch processing
echo ""
echo "3. Training with batch processing..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_batch \
  -dim 100 \
  -epoch 30 \
  -lr 0.1 \
  -minCount 1 \
  -batch 32 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Batch training successful"
else
    echo "✗ Batch training failed"
    exit 1
fi

# Test 4: Inference tests
echo ""
echo "4. Testing inference..."

# Positive sentiment
result=$(echo "This movie is absolutely amazing and wonderful" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__positive")
if [ -n "$result" ]; then
    echo "✓ Positive sentiment detected correctly"
else
    echo "✗ Positive sentiment detection failed"
    exit 1
fi

# Negative sentiment
result=$(echo "This movie is terrible and awful" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__negative")
if [ -n "$result" ]; then
    echo "✓ Negative sentiment detected correctly"
else
    echo "✗ Negative sentiment detection failed"
    exit 1
fi

# Neutral sentiment
result=$(echo "This movie is okay and average" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__neutral")
if [ -n "$result" ]; then
    echo "✓ Neutral sentiment detected correctly"
else
    echo "✗ Neutral sentiment detection failed"
    exit 1
fi

# Test 5: Attention modes
echo ""
echo "5. Testing attention modes..."

for mode in full simple none; do
    ../../gladtotext supervised \
      -input $TRAIN_DATA \
      -output ${MODEL_NAME}_${mode} \
      -dim 50 \
      -epoch 10 \
      -lr 0.1 \
      -minCount 1 \
      -attention $mode \
      > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        echo "✓ Attention mode '$mode' successful"
    else
        echo "✗ Attention mode '$mode' failed"
        exit 1
    fi
done

# Cleanup
rm -f ${MODEL_NAME}*.bin

echo ""
echo "=== All sentiment classification tests passed! ==="
