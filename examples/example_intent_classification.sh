#!/bin/bash

# Complete Example: Text vs Word vs Sentence-based Intent Classification
# This demonstrates the differences between three approaches

echo "=========================================="
echo "Intent Classification: 3 Approaches Demo"
echo "=========================================="
echo ""

# Create comprehensive training data
cat > intent_train.txt << 'EOF'
__book_flight book a flight to paris
__book_flight i want to book a flight
__book_flight reserve flight tickets
__book_flight book me a plane ticket
__book_flight flight booking to london
__book_flight can you book a flight
__book_flight i need to fly to tokyo
__book_flight book flight for tomorrow
__cancel_flight cancel my flight
__cancel_flight i want to cancel flight
__cancel_flight cancel the booking
__cancel_flight need to cancel my ticket
__cancel_flight cancel flight reservation
__cancel_flight please cancel my flight
__check_status what is my flight status
__check_status check flight status
__check_status flight status inquiry
__check_status status of my booking
__check_status where is my flight
__check_status is my flight on time
__greeting hello
__greeting hi there
__greeting good morning
__greeting hey how are you
__greeting greetings
__greeting hello friend
__farewell goodbye
__farewell bye bye
__farewell see you later
__farewell take care
__farewell farewell friend
__farewell see you soon
EOF

echo "Training data: 32 examples, 5 intents"
echo "  - __book_flight (8 examples)"
echo "  - __cancel_flight (6 examples)"
echo "  - __check_status (6 examples)"
echo "  - __greeting (6 examples)"
echo "  - __farewell (6 examples)"
echo ""

# Build the tools
echo "Building GLADtoTEXT..."
make gladtotext gladtotext-infer > /dev/null 2>&1
echo ""

# ============================================
# APPROACH 1: Text-level (Bag-of-Words)
# ============================================
echo "=========================================="
echo "APPROACH 1: Text-level (Bag-of-Words)"
echo "=========================================="
echo "Description: Treats text as unordered set of words"
echo "             No word order, no context"
echo ""

echo "Training text-level model..."
./gladtotext supervised \
  -input intent_train.txt \
  -output model_text \
  -dim 30 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 1 \
  > /dev/null 2>&1

echo "Model trained!"
echo ""

# ============================================
# APPROACH 2: Word-level (with subword features)
# ============================================
echo "=========================================="
echo "APPROACH 2: Word-level (Subword Features)"
echo "=========================================="
echo "Description: Each word encoded with char n-grams,"
echo "             grammar units, phonetic features"
echo "             Better OOV handling, still no order"
echo ""

echo "Training word-level model..."
./gladtotext supervised \
  -input intent_train.txt \
  -output model_word \
  -dim 30 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 1 \
  -minn 3 \
  -maxn 6 \
  > /dev/null 2>&1

echo "Model trained!"
echo ""

# ============================================
# APPROACH 3: Sentence-level (with attention)
# ============================================
echo "=========================================="
echo "APPROACH 3: Sentence-level (Attention)"
echo "=========================================="
echo "Description: Word order + context via self-attention"
echo "             Position encoding + Q/K/V attention"
echo "             Captures compositional meaning"
echo ""

echo "Training sentence-level model..."
./gladtotext supervised \
  -input intent_train.txt \
  -output model_sentence \
  -dim 30 \
  -epoch 20 \
  -lr 0.1 \
  -minCount 1 \
  -minn 3 \
  -maxn 6 \
  -sentence \
  > /dev/null 2>&1

echo "Model trained!"
echo ""

# ============================================
# TESTING: Compare all three approaches
# ============================================
echo "=========================================="
echo "TESTING: Comparing All Approaches"
echo "=========================================="
echo ""

# Test cases designed to show differences
test_cases=(
  "book a flight to paris"
  "flight book to paris"
  "cancel my flight"
  "flight cancel my"
  "check flight status"
  "status flight check"
  "hello friend"
  "friend hello"
  "goodbye see you"
  "i want to book"
  "book flight tomorrow"
  "cancel booking please"
)

echo "Test Case 1: Normal word order"
echo "Input: 'book a flight to paris'"
echo ""
echo "Text-level (bag-of-words):"
echo "book a flight to paris" | ./gladtotext-infer predict model_text.bin 2 2>/dev/null | head -2
echo ""
echo "Word-level (subword features):"
echo "book a flight to paris" | ./gladtotext-infer predict model_word.bin 2 2>/dev/null | head -2
echo ""
echo "Sentence-level (attention):"
echo "book a flight to paris" | ./gladtotext-infer predict model_sentence.bin 2 2>/dev/null | head -2
echo ""
echo "---"
echo ""

echo "Test Case 2: Reversed word order (should show difference)"
echo "Input: 'flight book to paris'"
echo ""
echo "Text-level (bag-of-words):"
echo "flight book to paris" | ./gladtotext-infer predict model_text.bin 2 2>/dev/null | head -2
echo ""
echo "Word-level (subword features):"
echo "flight book to paris" | ./gladtotext-infer predict model_word.bin 2 2>/dev/null | head -2
echo ""
echo "Sentence-level (attention):"
echo "flight book to paris" | ./gladtotext-infer predict model_sentence.bin 2 2>/dev/null | head -2
echo ""
echo "---"
echo ""

echo "Test Case 3: Cancel intent"
echo "Input: 'cancel my flight'"
echo ""
echo "Text-level:"
echo "cancel my flight" | ./gladtotext-infer predict model_text.bin 2 2>/dev/null | head -2
echo ""
echo "Word-level:"
echo "cancel my flight" | ./gladtotext-infer predict model_word.bin 2 2>/dev/null | head -2
echo ""
echo "Sentence-level:"
echo "cancel my flight" | ./gladtotext-infer predict model_sentence.bin 2 2>/dev/null | head -2
echo ""
echo "---"
echo ""

echo "Test Case 4: Reversed cancel (should confuse bag-of-words)"
echo "Input: 'flight cancel my'"
echo ""
echo "Text-level:"
echo "flight cancel my" | ./gladtotext-infer predict model_text.bin 2 2>/dev/null | head -2
echo ""
echo "Word-level:"
echo "flight cancel my" | ./gladtotext-infer predict model_word.bin 2 2>/dev/null | head -2
echo ""
echo "Sentence-level:"
echo "flight cancel my" | ./gladtotext-infer predict model_sentence.bin 2 2>/dev/null | head -2
echo ""
echo "---"
echo ""

echo "Test Case 5: Out-of-vocabulary word (typo)"
echo "Input: 'bok a flite to paris' (book → bok, flight → flite)"
echo ""
echo "Text-level (no subword features):"
echo "bok a flite to paris" | ./gladtotext-infer predict model_text.bin 2 2>/dev/null | head -2
echo ""
echo "Word-level (with subword features):"
echo "bok a flite to paris" | ./gladtotext-infer predict model_word.bin 2 2>/dev/null | head -2
echo ""
echo "Sentence-level (subword + attention):"
echo "bok a flite to paris" | ./gladtotext-infer predict model_sentence.bin 2 2>/dev/null | head -2
echo ""
echo "---"
echo ""

echo "Test Case 6: Greeting with order"
echo "Input: 'hello friend' vs 'friend hello'"
echo ""
echo "Text-level ('hello friend'):"
echo "hello friend" | ./gladtotext-infer predict model_text.bin 1 2>/dev/null | head -1
echo ""
echo "Text-level ('friend hello'):"
echo "friend hello" | ./gladtotext-infer predict model_text.bin 1 2>/dev/null | head -1
echo ""
echo "Sentence-level ('hello friend'):"
echo "hello friend" | ./gladtotext-infer predict model_sentence.bin 1 2>/dev/null | head -1
echo ""
echo "Sentence-level ('friend hello'):"
echo "friend hello" | ./gladtotext-infer predict model_sentence.bin 1 2>/dev/null | head -1
echo ""
echo "---"
echo ""

# ============================================
# COMPARISON SUMMARY
# ============================================
echo "=========================================="
echo "COMPARISON SUMMARY"
echo "=========================================="
echo ""

echo "1. TEXT-LEVEL (Bag-of-Words)"
echo "   Pros:"
echo "   - Fastest training and inference"
echo "   - Simplest implementation"
echo "   - Works well for keyword-based tasks"
echo ""
echo "   Cons:"
echo "   - No word order awareness"
echo "   - Poor OOV handling"
echo "   - 'book flight' = 'flight book'"
echo ""

echo "2. WORD-LEVEL (Subword Features)"
echo "   Pros:"
echo "   - Better OOV handling (char n-grams)"
echo "   - Handles typos and rare words"
echo "   - Grammar and phonetic features"
echo ""
echo "   Cons:"
echo "   - Still no word order"
echo "   - 'book flight' = 'flight book'"
echo "   - Slightly slower than text-level"
echo ""

echo "3. SENTENCE-LEVEL (Attention)"
echo "   Pros:"
echo "   - Word order awareness"
echo "   - Compositional meaning"
echo "   - Best for complex intents"
echo "   - 'book flight' ≠ 'flight book'"
echo ""
echo "   Cons:"
echo "   - Slower training/inference (1.5-2x)"
echo "   - Larger model size"
echo "   - Needs more training data"
echo ""

# Model sizes
echo "=========================================="
echo "MODEL SIZES"
echo "=========================================="
echo ""
ls -lh model_text.bin model_word.bin model_sentence.bin 2>/dev/null | awk '{print $9, $5}'
echo ""

# ============================================
# RECOMMENDATIONS
# ============================================
echo "=========================================="
echo "WHEN TO USE EACH APPROACH"
echo "=========================================="
echo ""

echo "Use TEXT-LEVEL when:"
echo "  - Simple keyword matching is sufficient"
echo "  - Speed is critical"
echo "  - Training data is limited"
echo "  - Example: 'pizza' → __order_food"
echo ""

echo "Use WORD-LEVEL when:"
echo "  - Need to handle typos and rare words"
echo "  - OOV words are common"
echo "  - Word order doesn't matter much"
echo "  - Example: 'piza' → __order_food (typo handling)"
echo ""

echo "Use SENTENCE-LEVEL when:"
echo "  - Word order affects meaning"
echo "  - Complex compositional intents"
echo "  - Multi-word expressions"
echo "  - Examples:"
echo "    - 'book flight' vs 'flight book'"
echo "    - 'not good' vs 'good'"
echo "    - 'how are you' (greeting, not question)"
echo ""

echo "=========================================="
echo "EXAMPLE COMPLETE"
echo "=========================================="
echo ""
echo "Models saved:"
echo "  - model_text.bin (text-level)"
echo "  - model_word.bin (word-level)"
echo "  - model_sentence.bin (sentence-level)"
echo ""
echo "Try your own examples:"
echo "  echo 'your text' | ./gladtotext-infer predict model_sentence.bin 1"
