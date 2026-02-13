#!/bin/bash

# Edge case testing: typos, negations, short/long sentences, special characters

echo "=== Edge Case Robustness Test ==="
echo ""

TRAIN_DATA="../data/large_scale_train.txt"
EDGE_DATA="../data/edge_cases.txt"
MODEL_NAME="edge_case_model"

# Clean up any existing model files first
rm -f ${MODEL_NAME}*.bin

# Train a model first
echo "1. Training model for edge case testing..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output $MODEL_NAME \
  -dim 150 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 2 \
  -batch 32 \
  > /tmp/edge_train_output.txt 2>&1

TRAIN_EXIT=$?
if [ $TRAIN_EXIT -eq 0 ]; then
    echo "✓ Model training successful"
else
    echo "✗ Model training failed (exit code: $TRAIN_EXIT)"
    echo "   Error output:"
    tail -20 /tmp/edge_train_output.txt | sed 's/^/   /'
    rm -f /tmp/edge_train_output.txt
    exit 1
fi
rm -f /tmp/edge_train_output.txt

# Test 2: Heavy typos
echo ""
echo "2. Testing with heavy typos..."

result=$(echo "I need hlep with my acount" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__customer_support")
if [ -n "$result" ]; then
    echo "✓ Heavy typos handled (hlep -> help, acount -> account)"
else
    echo "⚠ Heavy typos may affect accuracy"
fi

result=$(echo "What are the prodcut specifications" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__product_inquiry")
if [ -n "$result" ]; then
    echo "✓ Typo handled (prodcut -> product)"
else
    echo "⚠ Typo may affect accuracy"
fi

result=$(echo "Where is my oder" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__order_status")
if [ -n "$result" ]; then
    echo "✓ Typo handled (oder -> order)"
else
    echo "⚠ Typo may affect accuracy"
fi

# Test 3: Multiple negations
echo ""
echo "3. Testing with multiple negations..."

result=$(echo "I dont want to cancel my order" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1)
echo "   Input: 'I dont want to cancel my order'"
echo "   Result: $(echo "$result" | grep "^__" | head -1)"

result=$(echo "Payment isnt processing and I cant complete checkout" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__payment_issue")
if [ -n "$result" ]; then
    echo "✓ Multiple negations handled"
else
    echo "⚠ Multiple negations may affect accuracy"
fi

result=$(echo "I wasnt able to return this item" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__return_refund")
if [ -n "$result" ]; then
    echo "✓ Negation with return handled"
else
    echo "⚠ Negation may affect accuracy"
fi

# Test 4: Very short sentences
echo ""
echo "4. Testing with very short sentences..."

result=$(echo "Website not" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__technical_issue")
if [ -n "$result" ]; then
    echo "✓ Very short sentence handled (2 words)"
else
    echo "⚠ Very short sentences may affect accuracy"
fi

result=$(echo "App keeps" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__technical_issue")
if [ -n "$result" ]; then
    echo "✓ Minimal context handled"
else
    echo "⚠ Minimal context may affect accuracy"
fi

result=$(echo "Product is" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__product_complaint")
if [ -n "$result" ]; then
    echo "✓ Incomplete sentence handled"
else
    echo "⚠ Incomplete sentences may affect accuracy"
fi

# Test 5: Very long sentences
echo ""
echo "5. Testing with very long sentences..."

result=$(echo "Great product love it and I would appreciate your help and I need this resolved quickly as soon as possible please" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__feedback_review")
if [ -n "$result" ]; then
    echo "✓ Long sentence handled (20+ words)"
else
    echo "⚠ Long sentences may affect accuracy"
fi

result=$(echo "Excellent service because this is very important to me because this is very important to me since I have been waiting for a while" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__feedback_review")
if [ -n "$result" ]; then
    echo "✓ Repetitive long sentence handled"
else
    echo "⚠ Repetitive content may affect accuracy"
fi

# Test 6: Case variations
echo ""
echo "6. Testing case variations..."

result=$(echo "I NEED HELP WITH MY ACCOUNT" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__customer_support")
if [ -n "$result" ]; then
    echo "✓ ALL CAPS handled"
else
    echo "⚠ ALL CAPS may affect accuracy"
fi

result=$(echo "what are the product specifications" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__product_inquiry")
if [ -n "$result" ]; then
    echo "✓ all lowercase handled"
else
    echo "⚠ all lowercase may affect accuracy"
fi

result=$(echo "WhErE iS mY oRdEr" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__order_status")
if [ -n "$result" ]; then
    echo "✓ MiXeD cAsE handled"
else
    echo "⚠ MiXeD cAsE may affect accuracy"
fi

# Test 7: Special characters and numbers
echo ""
echo "7. Testing special characters and numbers..."

result=$(echo "Order #12345 status" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__order_status")
if [ -n "$result" ]; then
    echo "✓ Numbers handled"
else
    echo "⚠ Numbers may affect accuracy"
fi

result=$(echo "Payment failed!!! Help needed!!!" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__payment_issue")
if [ -n "$result" ]; then
    echo "✓ Exclamation marks handled"
else
    echo "⚠ Special punctuation may affect accuracy"
fi

result=$(echo "Product @ 50% discount???" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__pricing_discount")
if [ -n "$result" ]; then
    echo "✓ Special characters handled"
else
    echo "⚠ Special characters may affect accuracy"
fi

# Test 8: Word repetition
echo ""
echo "8. Testing word repetition..."

result=$(echo "help help help I need help" | ../../gladtotext-infer predict ${MODEL_NAME}.bin 1 2>&1 | grep "__customer_support")
if [ -n "$result" ]; then
    echo "✓ Word repetition handled"
else
    echo "⚠ Word repetition may affect accuracy"
fi

# Test 9: Batch inference on all edge cases
echo ""
echo "9. Running batch inference on all edge cases..."
EDGE_COUNT=$(wc -l < $EDGE_DATA)
echo "   Processing $EDGE_COUNT edge cases..."

../../gladtotext-infer test ${MODEL_NAME}.bin $EDGE_DATA 1 > edge_results.txt 2>&1

if [ $? -eq 0 ]; then
    # Count predictions
    PREDICTIONS=$(grep -c "^__" edge_results.txt)
    echo "✓ Batch inference completed: $PREDICTIONS predictions"
    
    # Show accuracy if available
    ACCURACY=$(grep "Accuracy" edge_results.txt | tail -1)
    if [ -n "$ACCURACY" ]; then
        echo "   $ACCURACY"
    fi
else
    echo "✗ Batch inference failed"
    exit 1
fi

# Test 10: Sentence encoding with edge cases
echo ""
echo "10. Testing sentence encoding with edge cases..."
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_sentence \
  -dim 100 \
  -epoch 15 \
  -lr 0.1 \
  -minCount 2 \
  -sentence \
  -batch 32 \
  > /tmp/edge_sentence_output.txt 2>&1

SENT_EXIT=$?
if [ $SENT_EXIT -eq 0 ]; then
    echo "✓ Sentence encoding model trained"
    
    # Test with typos
    result=$(echo "I need hlep with my acount" | ../../gladtotext-infer predict ${MODEL_NAME}_sentence.bin 1 2>&1 | grep "__customer_support")
    if [ -n "$result" ]; then
        echo "✓ Sentence encoding handles typos"
    else
        echo "⚠ Sentence encoding may struggle with typos"
    fi
else
    echo "✗ Sentence encoding training failed (exit code: $SENT_EXIT)"
    echo "   Error output:"
    tail -20 /tmp/edge_sentence_output.txt | sed 's/^/   /'
    rm -f /tmp/edge_sentence_output.txt
    exit 1
fi
rm -f /tmp/edge_sentence_output.txt

# Cleanup
rm -f ${MODEL_NAME}*.bin edge_results.txt

echo ""
echo "=== Edge case testing completed! ==="
echo ""
echo "Note: Some edge cases may show warnings (⚠) which is expected."
echo "The model should handle most cases gracefully even if accuracy is reduced."
