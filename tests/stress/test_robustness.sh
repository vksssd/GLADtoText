#!/bin/bash

# Robustness testing: typo tolerance, negation handling, length variations

echo "=== Robustness Testing ==="
echo ""

TRAIN_DATA="../data/large_scale_train.txt"
MODEL_NAME="robust_model"

# Clean up any existing model files first
rm -f ${MODEL_NAME}*.bin

# Train models with different configurations
echo "1. Training models for robustness testing..."

# Standard model
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_standard \
  -dim 150 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 2 \
  -batch 32 \
  > /tmp/robust_std_output.txt 2>&1

STD_EXIT=$?
if [ $STD_EXIT -eq 0 ]; then
    echo "✓ Standard model trained"
else
    echo "✗ Standard model training failed (exit code: $STD_EXIT)"
    echo "   Error output:"
    tail -20 /tmp/robust_std_output.txt | sed 's/^/   /'
    rm -f /tmp/robust_std_output.txt
    exit 1
fi
rm -f /tmp/robust_std_output.txt

# Sentence encoding model
../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output ${MODEL_NAME}_sentence \
  -dim 150 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 2 \
  -sentence \
  -batch 32 \
  > /tmp/robust_sent_output.txt 2>&1

SENT_EXIT=$?
if [ $SENT_EXIT -eq 0 ]; then
    echo "✓ Sentence encoding model trained"
else
    echo "✗ Sentence encoding model training failed (exit code: $SENT_EXIT)"
    echo "   Error output:"
    tail -20 /tmp/robust_sent_output.txt | sed 's/^/   /'
    rm -f /tmp/robust_sent_output.txt
    exit 1
fi
rm -f /tmp/robust_sent_output.txt

# Test 2: Typo tolerance levels
echo ""
echo "2. Testing typo tolerance..."

# Original
original="I need help with my account"
result=$(echo "$original" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Original: '$original'"
echo "   → $result"

# Light typos (1 typo)
typo1="I need hlep with my account"
result=$(echo "$typo1" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Light typo: '$typo1'"
echo "   → $result"

# Medium typos (2-3 typos)
typo2="I nede hlep with my acount"
result=$(echo "$typo2" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Medium typos: '$typo2'"
echo "   → $result"

# Heavy typos (4+ typos)
typo3="I nede hlpe wiht my acount"
result=$(echo "$typo3" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Heavy typos: '$typo3'"
echo "   → $result"

echo "✓ Typo tolerance tested (results may vary with typo severity)"

# Test 3: Negation handling
echo ""
echo "3. Testing negation handling..."

# Positive statement
pos="I want to return this product"
result=$(echo "$pos" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Positive: '$pos'"
echo "   → $result"

# Single negation
neg1="I dont want to return this product"
result=$(echo "$neg1" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Single negation: '$neg1'"
echo "   → $result"

# Double negation
neg2="I dont not want to return this product"
result=$(echo "$neg2" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Double negation: '$neg2'"
echo "   → $result"

# Complex negation
neg3="I wasnt unable to not return this product"
result=$(echo "$neg3" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Complex negation: '$neg3'"
echo "   → $result"

echo "✓ Negation handling tested"

# Test 4: Sentence length variations
echo ""
echo "4. Testing sentence length variations..."

# Very short (2-3 words)
short="Help needed"
result=$(echo "$short" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Very short (2 words): '$short'"
echo "   → $result"

# Short (4-6 words)
short2="I need help with account"
result=$(echo "$short2" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Short (5 words): '$short2'"
echo "   → $result"

# Medium (7-12 words)
medium="I need help with my account and password reset"
result=$(echo "$medium" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Medium (10 words): '$medium'"
echo "   → $result"

# Long (13-20 words)
long="I need help with my account because I cannot access my profile and need to reset my password"
result=$(echo "$long" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Long (18 words): '$long'"
echo "   → $result"

# Very long (20+ words)
verylong="I need help with my account because I cannot access my profile and need to reset my password as soon as possible because this is very important"
result=$(echo "$verylong" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Very long (26 words): '$verylong'"
echo "   → $result"

echo "✓ Length variation tested"

# Test 5: Word order variations
echo ""
echo "5. Testing word order robustness..."

# Standard order
std="What are the product specifications"
result=$(echo "$std" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Standard: '$std'"
echo "   → $result"

# Slightly shuffled
shuffle1="What are specifications the product"
result=$(echo "$shuffle1" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Shuffled: '$shuffle1'"
echo "   → $result"

# More shuffled
shuffle2="Product specifications what are the"
result=$(echo "$shuffle2" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   More shuffled: '$shuffle2'"
echo "   → $result"

echo "✓ Word order robustness tested"

# Test 6: Comparing bag-of-words vs sentence encoding
echo ""
echo "6. Comparing bag-of-words vs sentence encoding..."

test_sentences=(
    "I need help with my account"
    "My account needs help"
    "Help me with account issues"
    "Account help needed urgently"
)

echo "   Testing with bag-of-words model:"
for sent in "${test_sentences[@]}"; do
    result=$(echo "$sent" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
    echo "     '$sent' → $result"
done

echo ""
echo "   Testing with sentence encoding model:"
for sent in "${test_sentences[@]}"; do
    result=$(echo "$sent" | ../../gladtotext-infer predict ${MODEL_NAME}_sentence.bin 1 2>&1 | grep "^__" | head -1)
    echo "     '$sent' → $result"
done

echo "✓ Model comparison completed"

# Test 7: Special character robustness
echo ""
echo "7. Testing special character handling..."

# Punctuation
punct="Help!!! I need assistance!!!"
result=$(echo "$punct" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Heavy punctuation: '$punct'"
echo "   → $result"

# Numbers
nums="Order #12345 status check"
result=$(echo "$nums" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   With numbers: '$nums'"
echo "   → $result"

# Mixed special chars
mixed="Product @ 50% discount - limited time!!!"
result=$(echo "$mixed" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Mixed special chars: '$mixed'"
echo "   → $result"

echo "✓ Special character handling tested"

# Test 8: Case sensitivity
echo ""
echo "8. Testing case sensitivity..."

# Normal case
normal="I need help with my account"
result=$(echo "$normal" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Normal: '$normal'"
echo "   → $result"

# All caps
caps="I NEED HELP WITH MY ACCOUNT"
result=$(echo "$caps" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   ALL CAPS: '$caps'"
echo "   → $result"

# All lowercase
lower="i need help with my account"
result=$(echo "$lower" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   all lowercase: '$lower'"
echo "   → $result"

# Mixed case
mixcase="I nEeD hElP wItH mY aCcOuNt"
result=$(echo "$mixcase" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   MiXeD cAsE: '$mixcase'"
echo "   → $result"

echo "✓ Case sensitivity tested"

# Test 9: Repeated words
echo ""
echo "9. Testing repeated word handling..."

# Normal
normal="I need help"
result=$(echo "$normal" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Normal: '$normal'"
echo "   → $result"

# Some repetition
rep1="I need need help help"
result=$(echo "$rep1" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Some repetition: '$rep1'"
echo "   → $result"

# Heavy repetition
rep2="help help help help help"
result=$(echo "$rep2" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
echo "   Heavy repetition: '$rep2'"
echo "   → $result"

echo "✓ Repeated word handling tested"

# Test 10: Pronoun variations
echo ""
echo "10. Testing pronoun variations..."

pronouns=(
    "I need help with my account"
    "You need help with your account"
    "He needs help with his account"
    "She needs help with her account"
    "We need help with our account"
    "They need help with their account"
)

for sent in "${pronouns[@]}"; do
    result=$(echo "$sent" | ../../gladtotext-infer predict ${MODEL_NAME}_standard.bin 1 2>&1 | grep "^__" | head -1)
    echo "   '$sent' → $result"
done

echo "✓ Pronoun variation tested"

# Cleanup
rm -f ${MODEL_NAME}*.bin

echo ""
echo "=== All robustness tests completed! ==="
echo ""
echo "Summary:"
echo "  ✓ Typo tolerance (light to heavy)"
echo "  ✓ Negation handling (single to complex)"
echo "  ✓ Length variations (2 to 26+ words)"
echo "  ✓ Word order robustness"
echo "  ✓ Bag-of-words vs sentence encoding"
echo "  ✓ Special character handling"
echo "  ✓ Case sensitivity"
echo "  ✓ Repeated word handling"
echo "  ✓ Pronoun variations"
