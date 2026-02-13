#!/bin/bash

# Comprehensive test: Intent classification (8 classes)

echo "=== Intent Classification Test ==="
echo ""

TRAIN_DATA="../../tests/data/intent_classification.txt"
VALID_DATA="../../tests/data/intent_validation.txt"
MODEL_NAME="intent_model"

# Test 1: Basic training
echo "1. Training intent classifier (8 classes)..."
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

# Test 2: Training with validation and early stopping
echo ""
echo "2. Training with validation and early stopping..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -valid $VALID_DATA \
  -output ${MODEL_NAME}_val \
  -dim 100 \
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

# Test 3: Batch processing with different sizes
echo ""
echo "3. Testing different batch sizes..."

for batch in 1 16 32 64; do
    ../../gladtotext supervised \
      -input $TRAIN_DATA \
      -output ${MODEL_NAME}_batch${batch} \
      -dim 50 \
      -epoch 20 \
      -lr 0.1 \
      -minCount 1 \
      -batch $batch \
      > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        echo "✓ Batch size $batch successful"
    else
        echo "✗ Batch size $batch failed"
        exit 1
    fi
done

# Test 4: Inference tests for each intent
echo ""
echo "4. Testing inference for all intents..."

# Book flight
result=$(echo "I want to book a flight to New York" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__book_flight")
if [ -n "$result" ]; then
    echo "✓ book_flight intent detected"
else
    echo "✗ book_flight intent failed"
    exit 1
fi

# Book hotel
result=$(echo "I need a hotel room in Paris" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__book_hotel")
if [ -n "$result" ]; then
    echo "✓ book_hotel intent detected"
else
    echo "✗ book_hotel intent failed"
    exit 1
fi

# Cancel booking
result=$(echo "I want to cancel my reservation" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__cancel_booking")
if [ -n "$result" ]; then
    echo "✓ cancel_booking intent detected"
else
    echo "✗ cancel_booking intent failed"
    exit 1
fi

# Check status
result=$(echo "What is the status of my booking" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__check_status")
if [ -n "$result" ]; then
    echo "✓ check_status intent detected"
else
    echo "✗ check_status intent failed"
    exit 1
fi

# Get weather
result=$(echo "What is the weather like today" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__get_weather")
if [ -n "$result" ]; then
    echo "✓ get_weather intent detected"
else
    echo "✗ get_weather intent failed"
    exit 1
fi

# Greeting
result=$(echo "Hello how are you" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__greeting")
if [ -n "$result" ]; then
    echo "✓ greeting intent detected"
else
    echo "✗ greeting intent failed"
    exit 1
fi

# Farewell
result=$(echo "Goodbye see you later" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__farewell")
if [ -n "$result" ]; then
    echo "✓ farewell intent detected"
else
    echo "✗ farewell intent failed"
    exit 1
fi

# Help request
result=$(echo "I need help with something" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__help_request")
if [ -n "$result" ]; then
    echo "✓ help_request intent detected"
else
    echo "✗ help_request intent failed"
    exit 1
fi

# Test 5: Sentence encoding mode
echo ""
echo "5. Testing sentence encoding mode..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_sentence \
  -dim 50 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 1 \
  -sentence \
  > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Sentence encoding training successful"
    
    # Test inference with sentence encoding
    result=$(echo "I want to book a flight" | ../../gladtotext-infer predict ${MODEL_NAME}_sentence.bin 1 2>&1 | grep "sentence-encoding=ON")
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

# Cleanup
rm -f ${MODEL_NAME}*.bin

echo ""
echo "=== All intent classification tests passed! ==="
