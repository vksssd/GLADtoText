#!/bin/bash

set -e

echo "=========================================="
echo "GLADtoTEXT Transfer Learning Test"
echo "=========================================="
echo

# Clean up old files
rm -f test_*.txt test_*.bin

# Step 1: Create small unlabeled corpus
echo "Step 1: Creating unlabeled corpus..."
cat > test_corpus.txt << 'EOF'
the product quality is excellent
fast shipping and delivery
customer service was helpful
easy to use and setup
works perfectly as described
great value for money
highly recommend this item
satisfied with purchase
good build quality
reliable and durable
poor quality materials
slow shipping delays
unhelpful customer service
difficult to use
does not work properly
waste of money
do not recommend
disappointed with purchase
cheap build quality
unreliable and breaks
EOF

echo "✓ Created corpus with $(wc -l < test_corpus.txt) sentences"

# Step 2: Pretrain embeddings
echo
echo "Step 2: Pretraining embeddings..."
./gladtotext cbow \
  -input test_corpus.txt \
  -output test_pretrained \
  -dim 50 \
  -epoch 10 \
  -minCount 1 \
  -lr 0.05 \
  > /dev/null 2>&1

echo "✓ Pretrained embeddings saved ($(ls -lh test_pretrained.bin | awk '{print $5}'))"

# Step 3: Create labeled data
echo
echo "Step 3: Creating labeled dataset..."
cat > test_labels.txt << 'EOF'
__label__positive excellent quality and fast
__label__positive helpful service great value
__label__positive easy setup works perfectly
__label__positive satisfied recommend highly
__label__negative poor quality slow shipping
__label__negative unhelpful difficult to use
__label__negative does not work waste
__label__negative disappointed cheap unreliable
EOF

echo "✓ Created $(wc -l < test_labels.txt) labeled examples"

# Step 4: Train WITH pretrained embeddings
echo
echo "Step 4: Training classifier WITH pretrained embeddings..."
./gladtotext supervised \
  -input test_labels.txt \
  -output test_with_pretrain \
  -pretrained test_pretrained.bin \
  -epoch 20 \
  -lr 0.1 \
  > /dev/null 2>&1

echo "✓ Classifier with pretrain saved ($(ls -lh test_with_pretrain.bin | awk '{print $5}'))"

# Step 5: Train WITHOUT pretrained embeddings
echo
echo "Step 5: Training classifier WITHOUT pretrained embeddings..."
./gladtotext supervised \
  -input test_labels.txt \
  -output test_from_scratch \
  -dim 50 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 1 \
  > /dev/null 2>&1

echo "✓ Classifier from scratch saved ($(ls -lh test_from_scratch.bin | awk '{print $5}'))"

# Step 6: Test predictions
echo
echo "=========================================="
echo "Step 6: Testing Predictions"
echo "=========================================="
echo

test_cases=(
    "excellent quality fast shipping"
    "poor quality slow service"
    "great value highly recommend"
    "waste of money disappointed"
)

echo "Comparing predictions:"
echo

for test in "${test_cases[@]}"; do
    echo "Input: '$test'"
    
    # With pretrain
    result_pretrain=$(echo "$test" | ./gladtotext-infer predict test_with_pretrain.bin 1 2>/dev/null | head -1)
    label_pretrain=$(echo "$result_pretrain" | awk '{print $1}')
    score_pretrain=$(echo "$result_pretrain" | awk '{print $2}')
    
    # From scratch
    result_scratch=$(echo "$test" | ./gladtotext-infer predict test_from_scratch.bin 1 2>/dev/null | head -1)
    label_scratch=$(echo "$result_scratch" | awk '{print $1}')
    score_scratch=$(echo "$result_scratch" | awk '{print $2}')
    
    echo "  With pretrain:  $label_pretrain (score: $score_pretrain)"
    echo "  From scratch:   $label_scratch (score: $score_scratch)"
    
    # Check if both agree
    if [ "$label_pretrain" = "$label_scratch" ]; then
        echo "  ✓ Both models agree"
    else
        echo "  ⚠ Models disagree"
    fi
    echo
done

# Step 7: Summary
echo "=========================================="
echo "Summary"
echo "=========================================="
echo
echo "✓ Successfully demonstrated transfer learning workflow"
echo
echo "Files created:"
echo "  - test_pretrained.bin: Pretrained embeddings"
echo "  - test_with_pretrain.bin: Classifier using pretrained embeddings"
echo "  - test_from_scratch.bin: Baseline classifier"
echo
echo "Key observations:"
echo "  - Both models trained on same labeled data (8 examples)"
echo "  - Pretrained model leveraged 20 unlabeled sentences"
echo "  - Transfer learning provides better initialization"
echo
echo "Try your own predictions:"
echo "  echo 'your text' | ./gladtotext-infer predict test_with_pretrain.bin 1"
echo
echo "Clean up test files:"
echo "  rm -f test_*.txt test_*.bin"
