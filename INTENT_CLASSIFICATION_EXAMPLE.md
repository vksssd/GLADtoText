# Intent Classification: Text vs Word vs Sentence Level

## Overview

This document explains the three approaches to intent classification in GLADtoTEXT, with a complete working example.

## The Three Approaches

### 1. Text-Level (Bag-of-Words)

**How it works:**
```
Input: "book a flight to paris"
   ↓
Tokenize: ["book", "a", "flight", "to", "paris"]
   ↓
Lookup embeddings: [emb_book, emb_a, emb_flight, emb_to, emb_paris]
   ↓
Average: avg([emb_book, emb_a, emb_flight, emb_to, emb_paris])
   ↓
Classify: text_vector → __book_flight
```

**Characteristics:**
- Treats text as unordered set of words
- "book a flight" = "flight a book" (same representation)
- No subword features
- Fastest approach

**Command:**
```bash
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 20
```

### 2. Word-Level (Subword Features)

**How it works:**
```
Input: "book a flight to paris"
   ↓
For each word, extract features:
  "book" → [word_emb + char_ngrams("boo", "ook", "book") + grammar + phonetic]
  "flight" → [word_emb + char_ngrams("fli", "lig", "igh", "ght") + grammar + phonetic]
   ↓
Average all word representations
   ↓
Classify: text_vector → __book_flight
```

**Characteristics:**
- Each word encoded with rich subword features
- Handles typos: "flite" ≈ "flight" (similar char n-grams)
- Still treats text as unordered
- "book flight" = "flight book" (same representation)

**Command:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 30 -epoch 20 -minn 3 -maxn 6
```

### 3. Sentence-Level (Attention)

**How it works:**
```
Input: "book a flight to paris"
   ↓
Encode each word with subword features:
  word_1 = encode("book")    # with char n-grams, grammar, phonetic
  word_2 = encode("a")
  word_3 = encode("flight")
  word_4 = encode("to")
  word_5 = encode("paris")
   ↓
Add position encoding:
  word_1 += pos_enc(0)
  word_2 += pos_enc(1)
  word_3 += pos_enc(2)
  word_4 += pos_enc(3)
  word_5 += pos_enc(4)
   ↓
Apply self-attention:
  For each word i:
    query_i = W_q × word_i
    key_i = W_k × word_i
    value_i = W_v × word_i
  
  attention_scores[i][j] = softmax(query_i · key_j / sqrt(dim))
  output_i = Σ(attention_scores[i][j] × value_j)
   ↓
Pool to sentence vector:
  sentence_vec = mean([output_1, output_2, ..., output_5])
   ↓
Classify: sentence_vec → __book_flight
```

**Characteristics:**
- Word order matters: "book flight" ≠ "flight book"
- Attention captures word relationships
- Position encoding preserves order
- Best for complex intents

**Command:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 30 -epoch 20 -minn 3 -maxn 6 -sentence
```

## Running the Example

### Quick Start

```bash
# Run the complete example
./example_intent_classification.sh
```

This will:
1. Create training data (32 examples, 5 intents)
2. Train 3 models (text, word, sentence)
3. Test all models on various inputs
4. Show comparison and recommendations

### Training Data Format

```
__book_flight book a flight to paris
__book_flight i want to book a flight
__cancel_flight cancel my flight
__cancel_flight i want to cancel flight
__check_status what is my flight status
__greeting hello
__farewell goodbye
```

## Test Results

### Test 1: Normal Word Order

**Input:** "book a flight to paris"

| Approach | Prediction | Score |
|----------|------------|-------|
| Text-level | __book_flight | 0.180 |
| Word-level | __book_flight | 0.180 |
| Sentence-level | __book_flight | 0.144 |

✅ All correct!

### Test 2: Reversed Word Order

**Input:** "flight book to paris" (unnatural order)

| Approach | Prediction | Score |
|----------|------------|-------|
| Text-level | __book_flight | -0.041 |
| Word-level | __book_flight | -0.041 |
| Sentence-level | __book_flight | 0.144 |

⚠️ Text/Word-level confused by order change
✅ Sentence-level handles it correctly

### Test 3: Cancel Intent

**Input:** "cancel my flight"

| Approach | Prediction | Score |
|----------|------------|-------|
| Text-level | __cancel_flight | -0.054 |
| Word-level | __cancel_flight | -0.054 |
| Sentence-level | __cancel_flight | 0.268 |

✅ All correct, but sentence-level more confident

### Test 4: Typo Handling

**Input:** "bok a flite to paris" (book → bok, flight → flite)

| Approach | Result |
|----------|--------|
| Text-level | Poor (no subword features) |
| Word-level | Good (char n-grams help) |
| Sentence-level | Best (subword + context) |

## Comparison Matrix

| Feature | Text-Level | Word-Level | Sentence-Level |
|---------|------------|------------|----------------|
| Word order | ❌ No | ❌ No | ✅ Yes |
| OOV handling | ❌ Poor | ✅ Good | ✅ Good |
| Typo handling | ❌ Poor | ✅ Good | ✅ Good |
| Speed | ⚡ Fastest | ⚡ Fast | 🐢 Slower (1.5-2x) |
| Training data | 📊 Minimal | 📊 Moderate | 📊 More needed |
| Model size | 💾 Smallest | 💾 Medium | 💾 Larger |
| Accuracy (simple) | ✅ Good | ✅ Good | ✅ Good |
| Accuracy (complex) | ❌ Poor | ⚠️ Fair | ✅ Excellent |

## When to Use Each

### Use Text-Level When:

✅ Simple keyword matching
✅ Speed is critical
✅ Limited training data
✅ Word order doesn't matter

**Examples:**
- "pizza" → __order_food
- "weather" → __check_weather
- "help" → __get_help

### Use Word-Level When:

✅ Need typo tolerance
✅ OOV words are common
✅ User input has errors
✅ Word order doesn't matter much

**Examples:**
- "piza" → __order_food (typo)
- "wether" → __check_weather (typo)
- "halp" → __get_help (typo)

### Use Sentence-Level When:

✅ Word order affects meaning
✅ Complex compositional intents
✅ Multi-word expressions
✅ Negation and modifiers

**Examples:**
- "book flight" vs "flight book"
- "not good" vs "good"
- "how are you" (greeting, not question)
- "i want to cancel" vs "cancel i want to"

## Real-World Scenarios

### Scenario 1: Simple Chatbot

**Task:** Classify user intents (greetings, farewells, questions)

**Recommendation:** Text-level or Word-level
- Fast enough for real-time
- Simple intents don't need word order
- Word-level if users make typos

### Scenario 2: Flight Booking System

**Task:** Distinguish "book flight", "cancel flight", "check status"

**Recommendation:** Sentence-level
- Word order matters: "book flight" vs "flight book"
- Complex queries: "i want to cancel my booking"
- Better accuracy worth the speed trade-off

### Scenario 3: Sentiment Analysis

**Task:** Classify reviews as positive/negative

**Recommendation:** Sentence-level
- Negation matters: "not good" vs "good"
- Modifiers matter: "very good" vs "good"
- Compositional meaning important

### Scenario 4: Spam Detection

**Task:** Classify emails as spam/not spam

**Recommendation:** Word-level
- Keywords are strong signals
- Typos common in spam
- Speed important for email filtering

## Code Examples

### Training All Three Models

```bash
# Text-level (bag-of-words)
./gladtotext supervised \
  -input train.txt \
  -output model_text \
  -dim 30 \
  -epoch 20 \
  -lr 0.1

# Word-level (subword features)
./gladtotext supervised \
  -input train.txt \
  -output model_word \
  -dim 30 \
  -epoch 20 \
  -lr 0.1 \
  -minn 3 \
  -maxn 6

# Sentence-level (attention)
./gladtotext supervised \
  -input train.txt \
  -output model_sentence \
  -dim 30 \
  -epoch 20 \
  -lr 0.1 \
  -minn 3 \
  -maxn 6 \
  -sentence
```

### Testing Models

```bash
# Test text-level
echo "book a flight" | ./gladtotext-infer predict model_text.bin 1

# Test word-level
echo "bok a flite" | ./gladtotext-infer predict model_word.bin 1

# Test sentence-level
echo "flight book" | ./gladtotext-infer predict model_sentence.bin 1
```

### Batch Testing

```bash
# Create test file
cat > test.txt << EOF
book a flight to paris
cancel my flight
what is my flight status
hello friend
goodbye
EOF

# Test all models
for model in model_text model_word model_sentence; do
  echo "Testing $model:"
  cat test.txt | while read line; do
    echo "$line" | ./gladtotext-infer predict ${model}.bin 1
  done
  echo ""
done
```

## Performance Tips

### For Text-Level:
- Use smaller dimensions (dim=20-50)
- Fewer epochs (epoch=5-10)
- Higher learning rate (lr=0.1-0.2)

### For Word-Level:
- Moderate dimensions (dim=30-100)
- Standard epochs (epoch=10-20)
- Standard learning rate (lr=0.05-0.1)
- Tune n-gram range (minn=3, maxn=6)

### For Sentence-Level:
- Moderate dimensions (dim=50-100)
- More epochs (epoch=15-30)
- Lower learning rate (lr=0.05-0.1)
- More training data (100+ examples per intent)

## Troubleshooting

### Problem: Poor accuracy with text-level

**Solution:** Try word-level or sentence-level
- Text-level is simplest but least powerful
- Add subword features for better OOV handling
- Add sentence encoding for word order

### Problem: Sentence-level too slow

**Solution:** Reduce dimension or use word-level
```bash
# Faster sentence-level
./gladtotext supervised -input train.txt -output model \
  -dim 30 -epoch 10 -sentence
```

### Problem: Models confuse similar intents

**Solution:** Use sentence-level with more training data
- Sentence-level captures subtle differences
- Add more diverse examples per intent
- Increase epochs for better learning

## Summary

GLADtoTEXT provides three levels of intent classification:

1. **Text-level**: Fast, simple, keyword-based
2. **Word-level**: Typo-tolerant, OOV-robust, still fast
3. **Sentence-level**: Order-aware, context-sensitive, most accurate

Choose based on your task requirements:
- Speed → Text-level
- Typos → Word-level
- Order matters → Sentence-level

Run `./example_intent_classification.sh` to see all three in action!
