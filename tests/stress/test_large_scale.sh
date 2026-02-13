#!/bin/bash

# Large-scale test: 10K training examples with 12 classes

echo "=== Large-Scale Classification Test (10K examples, 12 classes) ==="
echo ""

TRAIN_DATA="../data/large_scale_train.txt"
VALID_DATA="../data/large_scale_validation.txt"
MODEL_NAME="large_scale_model"

# Clean up any existing model files first
rm -f ${MODEL_NAME}*.bin

# Count examples
TRAIN_COUNT=$(wc -l < $TRAIN_DATA)
VALID_COUNT=$(wc -l < $VALID_DATA)
echo "Training examples: $TRAIN_COUNT"
echo "Validation examples: $VALID_COUNT"
echo ""

# Test 1: Basic training with large dataset
echo "1. Training on large-scale dataset (12 classes)..."
echo "   This may take a few minutes..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output $MODEL_NAME \
  -dim 150 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 2 \
  2>&1 | tail -5

if [ $? -eq 0 ]; then
    echo "✓ Large-scale training successful"
else
    echo "✗ Large-scale training failed"
    exit 1
fi

# Test 2: Training with validation set
echo ""
echo "2. Training with validation and early stopping..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -valid $VALID_DATA \
  -output ${MODEL_NAME}_val \
  -dim 150 \
  -epoch 30 \
  -lr 0.1 \
  -minCount 2 \
  -early-stop \
  -patience 5 \
  2>&1 | tail -10

if [ $? -eq 0 ]; then
    echo "✓ Validation training successful"
else
    echo "✗ Validation training failed"
    exit 1
fi

# Test 3: Batch processing for speed
echo ""
echo "3. Testing batch processing (batch=64)..."
time ../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_batch \
  -dim 150 \
  -epoch 15 \
  -lr 0.1 \
  -minCount 2 \
  -batch 64 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Batch processing successful"
else
    echo "✗ Batch processing failed"
    exit 1
fi

# Test 4: Optimized configuration (batch + simple attention)
echo ""
echo "4. Testing optimized configuration..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_opt \
  -dim 150 \
  -epoch 15 \
  -lr 0.1 \
  -minCount 2 \
  -batch 64 \
  -attention simple \
  > /tmp/test4_output.txt 2>&1

TEST4_EXIT=$?
if [ $TEST4_EXIT -eq 0 ]; then
    echo "✓ Optimized training successful"
else
    echo "✗ Optimized training failed (exit code: $TEST4_EXIT)"
    echo "   Error output:"
    tail -10 /tmp/test4_output.txt | sed 's/^/   /'
    rm -f /tmp/test4_output.txt
    exit 1
fi
rm -f /tmp/test4_output.txt

# Test 5: Inference tests for all 12 categories
echo ""
echo "5. Testing inference for all 12 categories..."

# Customer support
result=$(echo "I need help with my account" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__customer_support")
if [ -n "$result" ]; then
    echo "✓ customer_support detected"
else
    echo "✗ customer_support failed"
    exit 1
fi

# Product inquiry
result=$(echo "What are the product specifications" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__product_inquiry")
if [ -n "$result" ]; then
    echo "✓ product_inquiry detected"
else
    echo "✗ product_inquiry failed"
    exit 1
fi

# Order status
result=$(echo "Where is my order" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__order_status")
if [ -n "$result" ]; then
    echo "✓ order_status detected"
else
    echo "✗ order_status failed"
    exit 1
fi

# Payment issue
result=$(echo "Payment failed on my credit card" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__payment_issue")
if [ -n "$result" ]; then
    echo "✓ payment_issue detected"
else
    echo "✗ payment_issue failed"
    exit 1
fi

# Return refund
result=$(echo "I want to return this product" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__return_refund")
if [ -n "$result" ]; then
    echo "✓ return_refund detected"
else
    echo "✗ return_refund failed"
    exit 1
fi

# Account management
result=$(echo "Change my password" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__account_management")
if [ -n "$result" ]; then
    echo "✓ account_management detected"
else
    echo "✗ account_management failed"
    exit 1
fi

# Technical issue
result=$(echo "Website not loading" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__technical_issue")
if [ -n "$result" ]; then
    echo "✓ technical_issue detected"
else
    echo "✗ technical_issue failed"
    exit 1
fi

# Shipping delivery
result=$(echo "Express shipping options" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__shipping_delivery")
if [ -n "$result" ]; then
    echo "✓ shipping_delivery detected"
else
    echo "✗ shipping_delivery failed"
    exit 1
fi

# Product complaint
result=$(echo "Item arrived damaged" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__product_complaint")
if [ -n "$result" ]; then
    echo "✓ product_complaint detected"
else
    echo "✗ product_complaint failed"
    exit 1
fi

# Feedback review
result=$(echo "Great product love it" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__feedback_review")
if [ -n "$result" ]; then
    echo "✓ feedback_review detected"
else
    echo "✗ feedback_review failed"
    exit 1
fi

# Pricing discount
result=$(echo "Do you have any sale information" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__pricing_discount")
if [ -n "$result" ]; then
    echo "✓ pricing_discount detected"
else
    echo "✗ pricing_discount failed"
    exit 1
fi

# General inquiry
result=$(echo "What are your business hours" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__general_inquiry")
if [ -n "$result" ]; then
    echo "✓ general_inquiry detected"
else
    echo "✗ general_inquiry failed"
    exit 1
fi

# Test 6: Sentence encoding mode
echo ""
echo "6. Testing sentence encoding mode..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_sentence \
  -dim 100 \
  -epoch 15 \
  -lr 0.1 \
  -minCount 2 \
  -sentence \
  -batch 32 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Sentence encoding training successful"
    
    # Test inference
    result=$(echo "I need help with my account" | ../../gladtotext-infer predict ${MODEL_NAME}_sentence.bin 1 2>&1 | grep "sentence-encoding=ON")
    if [ -n "$result" ]; then
        echo "✓ Sentence encoding inference working"
    else
        echo "✗ Sentence encoding inference failed"
        exit 1
    fi
else
    echo "✗ Sentence encoding training failed"
    exit 1
fi

# Test 7: Model size check
echo ""
echo "7. Checking model file sizes..."
for model in ${MODEL_NAME}*.bin; do
    if [ -f "$model" ]; then
        size=$(du -h "$model" | cut -f1)
        echo "   $model: $size"
    fi
done

# Cleanup
rm -f ${MODEL_NAME}*.bin

echo ""
echo "=== All large-scale tests passed! ==="
