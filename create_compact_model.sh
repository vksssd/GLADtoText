#!/bin/bash

set -e

echo "=========================================="
echo "Creating Compact Intent Classifier"
echo "Target: 100-200KB (FastText-comparable)"
echo "=========================================="
echo

# Clean up
rm -f compact_*.txt compact_*.bin

# Step 1: Create intent training data
echo "Step 1: Creating intent training data..."
cat > compact_intents.txt << 'INTENTS'
__label__book_flight book a flight to paris
__label__book_flight i want to fly to london tomorrow
__label__book_flight reserve flight tickets to new york
__label__book_flight schedule a trip to tokyo next week
__label__book_hotel find a hotel in rome
__label__book_hotel i need accommodation in berlin
__label__book_hotel reserve a room in madrid
__label__book_hotel book hotel for two nights
__label__cancel_booking cancel my reservation
__label__cancel_booking i want to cancel my flight
__label__cancel_booking remove my booking
__label__cancel_booking delete my reservation please
__label__check_status what is my booking status
__label__check_status show my reservation details
__label__check_status check my flight information
__label__check_status where is my booking
INTENTS

echo "✓ Created $(wc -l < compact_intents.txt) training examples"

# Step 2: Train compact model with optimized parameters
echo
echo "Step 2: Training compact model..."
echo "Parameters optimized for size:"
echo "  - Small dimension: 20"
echo "  - Small bucket: 10000"
echo "  - Min n-gram: 0 (disable char n-grams for size)"
echo "  - Max n-gram: 0"

./gladtotext supervised \
  -input compact_intents.txt \
  -output compact_intent_classifier \
  -dim 20 \
  -epoch 50 \
  -lr 0.2 \
  -minCount 1 \
  -minn 0 \
  -maxn 0 \
  -bucket 10000

echo
echo "✓ Model trained"

# Check size
size=$(ls -lh compact_intent_classifier.bin | awk '{print $5}')
size_bytes=$(ls -l compact_intent_classifier.bin | awk '{print $5}')
size_kb=$((size_bytes / 1024))

echo
echo "=========================================="
echo "Model Size Analysis"
echo "=========================================="
echo "File size: $size ($size_kb KB)"
echo

if [ $size_kb -lt 200 ]; then
    echo "✓ SUCCESS: Model is under 200KB!"
elif [ $size_kb -lt 500 ]; then
    echo "⚠ Model is larger than target but acceptable"
else
    echo "✗ Model is too large, need more optimization"
fi

# Step 3: Test predictions
echo
echo "=========================================="
echo "Testing Predictions"
echo "=========================================="
echo

test_cases=(
    "book flight to paris"
    "find hotel in rome"
    "cancel my booking"
    "check reservation status"
)

for test in "${test_cases[@]}"; do
    echo "Input: '$test'"
    result=$(echo "$test" | ./gladtotext-infer predict compact_intent_classifier.bin 1 2>/dev/null | head -1)
    echo "  → $result"
    echo
done

echo "=========================================="
echo "Summary"
echo "=========================================="
echo
echo "Model: compact_intent_classifier.bin"
echo "Size: $size ($size_kb KB)"
echo "Intents: 4 classes"
echo "Training examples: 16"
echo
echo "Size breakdown:"
echo "  - Vocabulary: $(echo "$test" | ./gladtotext-infer predict compact_intent_classifier.bin 1 2>&1 | grep "Model loaded" | awk '{print $3}')"
echo "  - Dimension: 20"
echo "  - Bucket: 10000 (char n-grams disabled)"
echo
echo "To use:"
echo "  echo 'book flight to london' | ./gladtotext-infer predict compact_intent_classifier.bin 1"
