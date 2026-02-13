#!/bin/bash

# Comprehensive test: Topic classification (8 classes)

echo "=== Topic Classification Test ==="
echo ""

TRAIN_DATA="../../tests/data/topic_classification.txt"
VALID_DATA="../../tests/data/topic_validation.txt"
MODEL_NAME="topic_model"

# Test 1: Basic training with 8 classes
echo "1. Training topic classifier (8 classes)..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output $MODEL_NAME \
  -dim 150 \
  -epoch 40 \
  -lr 0.1 \
  -minCount 1 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Training successful"
else
    echo "✗ Training failed"
    exit 1
fi

# Test 2: Training with validation
echo ""
echo "2. Training with validation..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -valid $VALID_DATA \
  -output ${MODEL_NAME}_val \
  -dim 150 \
  -epoch 50 \
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

# Test 3: Optimized training (batch + attention)
echo ""
echo "3. Training with optimizations..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_optimized \
  -dim 100 \
  -epoch 30 \
  -lr 0.1 \
  -minCount 1 \
  -batch 64 \
  -attention simple \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Optimized training successful"
else
    echo "✗ Optimized training failed"
    exit 1
fi

# Test 4: Inference tests for all topics
echo ""
echo "4. Testing inference for all topics..."

# Sports
result=$(echo "The basketball game was very exciting" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__sports")
if [ -n "$result" ]; then
    echo "✓ sports topic detected"
else
    echo "✗ sports topic failed"
    exit 1
fi

# Technology
result=$(echo "New artificial intelligence breakthrough announced" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__technology")
if [ -n "$result" ]; then
    echo "✓ technology topic detected"
else
    echo "✗ technology topic failed"
    exit 1
fi

# Politics
result=$(echo "Election results show close race" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__politics")
if [ -n "$result" ]; then
    echo "✓ politics topic detected"
else
    echo "✗ politics topic failed"
    exit 1
fi

# Business
result=$(echo "Stock market reaches new high" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__business")
if [ -n "$result" ]; then
    echo "✓ business topic detected"
else
    echo "✗ business topic failed"
    exit 1
fi

# Science
result=$(echo "Researchers discover new species" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__science")
if [ -n "$result" ]; then
    echo "✓ science topic detected"
else
    echo "✗ science topic failed"
    exit 1
fi

# Entertainment
result=$(echo "New movie breaks box office records" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__entertainment")
if [ -n "$result" ]; then
    echo "✓ entertainment topic detected"
else
    echo "✗ entertainment topic failed"
    exit 1
fi

# Health
result=$(echo "Medical breakthrough offers hope" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__health")
if [ -n "$result" ]; then
    echo "✓ health topic detected"
else
    echo "✗ health topic failed"
    exit 1
fi

# Education
result=$(echo "University announces new program" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__education")
if [ -n "$result" ]; then
    echo "✓ education topic detected"
else
    echo "✗ education topic failed"
    exit 1
fi

# Test 5: Different configurations
echo ""
echo "5. Testing different configurations..."

# High dimension
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_highdim \
  -dim 200 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 1 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ High dimension (200) successful"
else
    echo "✗ High dimension failed"
    exit 1
fi

# Low dimension
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_lowdim \
  -dim 50 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 1 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Low dimension (50) successful"
else
    echo "✗ Low dimension failed"
    exit 1
fi

# Cleanup
rm -f ${MODEL_NAME}*.bin

echo ""
echo "=== All topic classification tests passed! ==="
