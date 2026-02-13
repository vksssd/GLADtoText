#!/bin/bash

# Stress testing: memory usage, concurrent inference, large batches

echo "=== Stress Testing ==="
echo ""

TRAIN_DATA="../data/large_scale_train.txt"
VALID_DATA="../data/large_scale_validation.txt"
MODEL_NAME="stress_model"

# Clean up any existing model files first
rm -f ${MODEL_NAME}*.bin

# Test 1: Memory usage with large dimensions
echo "1. Testing memory usage with large dimensions..."

echo "   Training with dim=300..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_300 \
  -dim 300 \
  -epoch 10 \
  -lr 0.1 \
  -minCount 2 \
  -batch 32 \
  > /tmp/stress_300_output.txt 2>&1

TEST_EXIT=$?
if [ $TEST_EXIT -eq 0 ]; then
    size=$(du -h ${MODEL_NAME}_300.bin | cut -f1)
    echo "✓ dim=300 successful (model size: $size)"
else
    echo "✗ dim=300 failed (exit code: $TEST_EXIT)"
    echo "   Error output:"
    tail -20 /tmp/stress_300_output.txt | sed 's/^/   /'
    rm -f /tmp/stress_300_output.txt
    exit 1
fi
rm -f /tmp/stress_300_output.txt

echo "   Training with dim=500..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_500 \
  -dim 500 \
  -epoch 10 \
  -lr 0.1 \
  -minCount 2 \
  -batch 32 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    size=$(du -h ${MODEL_NAME}_500.bin | cut -f1)
    echo "✓ dim=500 successful (model size: $size)"
else
    echo "✗ dim=500 failed"
    exit 1
fi

# Test 2: Large batch sizes
echo ""
echo "2. Testing large batch sizes..."

for batch in 64 128 256; do
    echo "   Testing batch=$batch..."
    ../../gladtotext supervised \
      -input $TRAIN_DATA \
      -output ${MODEL_NAME}_batch${batch} \
      -dim 100 \
      -epoch 10 \
      -lr 0.1 \
      -minCount 2 \
      -batch $batch \
      > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        echo "✓ batch=$batch successful"
    else
        echo "✗ batch=$batch failed"
        exit 1
    fi
done

# Test 3: Many epochs
echo ""
echo "3. Testing extended training (100 epochs)..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -valid $VALID_DATA \
  -output ${MODEL_NAME}_long \
  -dim 100 \
  -epoch 100 \
  -lr 0.1 \
  -minCount 2 \
  -batch 32 \
  -early-stop \
  -patience 10 \
  2>&1 | tail -5

if [ $? -eq 0 ]; then
    echo "✓ Extended training successful (early stopping may have triggered)"
else
    echo "✗ Extended training failed"
    exit 1
fi

# Test 4: Concurrent inference
echo ""
echo "4. Testing concurrent inference (10 parallel processes)..."

# Train a model for inference
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_infer \
  -dim 150 \
  -epoch 15 \
  -lr 0.1 \
  -minCount 2 \
  -batch 32 \
  > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "✗ Model training failed"
    exit 1
fi

# Run 10 concurrent inference processes
PIDS=()
for i in {1..10}; do
    (
        for j in {1..100}; do
            echo "I need help with my account" | ../../gladtotext-infer predict ${MODEL_NAME}_infer.bin 1 > /dev/null 2>&1
        done
    ) &
    PIDS+=($!)
done

# Wait for all processes
FAILED=0
for pid in ${PIDS[@]}; do
    wait $pid
    if [ $? -ne 0 ]; then
        FAILED=1
    fi
done

if [ $FAILED -eq 0 ]; then
    echo "✓ Concurrent inference successful (10 processes × 100 predictions)"
else
    echo "✗ Some concurrent processes failed"
    exit 1
fi

# Test 5: Rapid sequential inference
echo ""
echo "5. Testing rapid sequential inference (1000 predictions)..."

start_time=$(date +%s)
for i in {1..1000}; do
    echo "I need help with my account" | ../../gladtotext-infer predict ${MODEL_NAME}_infer.bin 1 > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "✗ Inference failed at iteration $i"
        exit 1
    fi
done
end_time=$(date +%s)
duration=$((end_time - start_time))

echo "✓ 1000 predictions completed in ${duration}s"
echo "   Average: $((1000 / duration)) predictions/second"

# Test 6: Batch inference with large file
echo ""
echo "6. Testing batch inference on large dataset..."

../../gladtotext-infer test ${MODEL_NAME}_infer.bin $TRAIN_DATA 1 > batch_results.txt 2>&1

if [ $? -eq 0 ]; then
    PREDICTIONS=$(grep -c "^__" batch_results.txt)
    echo "✓ Batch inference successful: $PREDICTIONS predictions"
    
    # Show accuracy
    ACCURACY=$(grep "Accuracy" batch_results.txt | tail -1)
    if [ -n "$ACCURACY" ]; then
        echo "   $ACCURACY"
    fi
else
    echo "✗ Batch inference failed"
    exit 1
fi

# Test 7: Memory stress with sentence encoding
echo ""
echo "7. Testing memory with sentence encoding..."

../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_sentence_large \
  -dim 200 \
  -epoch 15 \
  -lr 0.1 \
  -minCount 2 \
  -sentence \
  -batch 64 \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    size=$(du -h ${MODEL_NAME}_sentence_large.bin | cut -f1)
    echo "✓ Large sentence encoding model successful (size: $size)"
else
    echo "✗ Large sentence encoding model failed"
    exit 1
fi

# Test 8: Stress test with all features enabled
echo ""
echo "8. Testing with all features enabled..."

../../gladtotext supervised \
  -input $TRAIN_DATA \
  -valid $VALID_DATA \
  -output ${MODEL_NAME}_full \
  -dim 200 \
  -epoch 30 \
  -lr 0.1 \
  -minCount 2 \
  -sentence \
  -batch 64 \
  -early-stop \
  -patience 5 \
  2>&1 | tail -10

if [ $? -eq 0 ]; then
    echo "✓ Full feature stress test successful"
else
    echo "✗ Full feature stress test failed"
    exit 1
fi

# Test 9: Model file integrity
echo ""
echo "9. Testing model file integrity..."

# Save and reload multiple times
for i in {1..5}; do
    ../../gladtotext supervised \
      -input $TRAIN_DATA \
      -output ${MODEL_NAME}_integrity_${i} \
      -dim 100 \
      -epoch 5 \
      -lr 0.1 \
      -minCount 2 \
      -batch 32 \
      > /dev/null 2>&1
    
    if [ $? -ne 0 ]; then
        echo "✗ Model save failed at iteration $i"
        exit 1
    fi
    
    # Test inference
    result=$(echo "I need help" | ../../gladtotext-infer predict ${MODEL_NAME}_integrity_${i}.bin 1 2>&1 | grep "^__")
    if [ -z "$result" ]; then
        echo "✗ Model load failed at iteration $i"
        exit 1
    fi
done

echo "✓ Model integrity verified (5 save/load cycles)"

# Cleanup
rm -f ${MODEL_NAME}*.bin batch_results.txt

echo ""
echo "=== All stress tests passed! ==="
echo ""
echo "Summary:"
echo "  ✓ Large dimensions (300, 500)"
echo "  ✓ Large batches (64, 128, 256)"
echo "  ✓ Extended training (100 epochs)"
echo "  ✓ Concurrent inference (10 processes)"
echo "  ✓ Rapid inference (1000 predictions)"
echo "  ✓ Batch inference on large dataset"
echo "  ✓ Memory stress with sentence encoding"
echo "  ✓ All features combined"
echo "  ✓ Model file integrity"
