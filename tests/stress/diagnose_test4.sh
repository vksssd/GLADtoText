#!/bin/bash

# Diagnostic script to identify why test 4 is failing

echo "=== Diagnosing Test 4 Failure ==="
echo ""

cd "$(dirname "$0")"

TRAIN_DATA="../data/large_scale_train.txt"
MODEL_NAME="diagnostic_model"

# Check if binaries exist
echo "1. Checking binaries..."
if [ -f "../../gladtotext" ]; then
    echo "✓ gladtotext binary exists"
else
    echo "✗ gladtotext binary NOT found"
    echo "   Run 'make' in the root directory first"
    exit 1
fi

if [ -f "../../gladtotext-infer" ]; then
    echo "✓ gladtotext-infer binary exists"
else
    echo "✗ gladtotext-infer binary NOT found"
    exit 1
fi

# Check if data files exist
echo ""
echo "2. Checking data files..."
if [ -f "$TRAIN_DATA" ]; then
    lines=$(wc -l < $TRAIN_DATA)
    echo "✓ Training data exists ($lines lines)"
else
    echo "✗ Training data NOT found: $TRAIN_DATA"
    exit 1
fi

# Test basic training
echo ""
echo "3. Testing basic training (2 epochs)..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_basic \
  -dim 50 \
  -epoch 2 \
  -lr 0.1 \
  -minCount 2 \
  2>&1 | tail -5

if [ $? -eq 0 ]; then
    echo "✓ Basic training works"
    rm -f ${MODEL_NAME}_basic.bin
else
    echo "✗ Basic training failed"
    exit 1
fi

# Test with batch
echo ""
echo "4. Testing with batch=64..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_batch \
  -dim 50 \
  -epoch 2 \
  -lr 0.1 \
  -minCount 2 \
  -batch 64 \
  2>&1 | tail -5

if [ $? -eq 0 ]; then
    echo "✓ Batch training works"
    rm -f ${MODEL_NAME}_batch.bin
else
    echo "✗ Batch training failed"
    exit 1
fi

# Test with attention simple
echo ""
echo "5. Testing with -attention simple..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_attention \
  -dim 50 \
  -epoch 2 \
  -lr 0.1 \
  -minCount 2 \
  -attention simple \
  2>&1 | tail -10

if [ $? -eq 0 ]; then
    echo "✓ Attention simple works"
    rm -f ${MODEL_NAME}_attention.bin
else
    echo "✗ Attention simple failed"
    echo ""
    echo "Full output:"
    ../../gladtotext supervised \
      -input $TRAIN_DATA \
      -output ${MODEL_NAME}_attention \
      -dim 50 \
      -epoch 2 \
      -lr 0.1 \
      -minCount 2 \
      -attention simple \
      2>&1
    exit 1
fi

# Test with batch + attention simple (the failing combination)
echo ""
echo "6. Testing with batch=64 + attention simple..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_combined \
  -dim 50 \
  -epoch 2 \
  -lr 0.1 \
  -minCount 2 \
  -batch 64 \
  -attention simple \
  2>&1 | tail -10

if [ $? -eq 0 ]; then
    echo "✓ Combined (batch + attention) works"
    rm -f ${MODEL_NAME}_combined.bin
else
    echo "✗ Combined (batch + attention) failed"
    echo ""
    echo "Full output:"
    ../../gladtotext supervised \
      -input $TRAIN_DATA \
      -output ${MODEL_NAME}_combined \
      -dim 50 \
      -epoch 2 \
      -lr 0.1 \
      -minCount 2 \
      -batch 64 \
      -attention simple \
      2>&1
    exit 1
fi

# Check disk space
echo ""
echo "7. Checking disk space..."
df -h . | tail -1

# Check memory
echo ""
echo "8. Checking available memory..."
if command -v free &> /dev/null; then
    free -h | grep "Mem:"
elif command -v vm_stat &> /dev/null; then
    # macOS
    vm_stat | grep "Pages free"
fi

echo ""
echo "=== Diagnosis Complete ==="
echo ""
echo "If all tests passed, the issue may be:"
echo "  - Insufficient disk space for large models"
echo "  - Insufficient memory for large dimensions"
echo "  - Timeout or resource limits"
echo "  - Stale model files interfering"
