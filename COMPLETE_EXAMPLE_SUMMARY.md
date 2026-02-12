# Complete Intent Classification Example - Summary

## What Was Created

A comprehensive example demonstrating **three approaches** to intent classification:

1. **Text-Level** (Bag-of-Words)
2. **Word-Level** (Subword Features)
3. **Sentence-Level** (Attention)

## Files Created

### 1. `example_intent_classification.sh`
**Purpose:** Complete runnable example  
**What it does:**
- Creates training data (32 examples, 5 intents)
- Trains 3 models (text, word, sentence)
- Tests all models on 6 test cases
- Shows detailed comparison
- Provides recommendations

**Run it:**
```bash
./example_intent_classification.sh
```

### 2. `INTENT_CLASSIFICATION_EXAMPLE.md`
**Purpose:** Comprehensive documentation  
**Contains:**
- Detailed explanation of each approach
- Visual diagrams of how each works
- Test results and comparisons
- When to use each approach
- Real-world scenarios
- Code examples
- Troubleshooting guide

### 3. `QUICK_REFERENCE.md`
**Purpose:** Quick lookup guide  
**Contains:**
- Command cheat sheet
- Parameter guide
- Common examples
- Troubleshooting tips
- Performance tips

## The Three Approaches Explained

### Text-Level (Bag-of-Words)

**What it is:**
- Simplest approach
- Treats text as unordered set of words
- No subword features
- No word order

**How it works:**
```
"book a flight" → [emb_book, emb_a, emb_flight] → average → classify
```

**When to use:**
- Simple keyword matching
- Speed is critical
- Limited training data

**Command:**
```bash
./gladtotext supervised -input train.txt -output model -dim 30 -epoch 10
```

### Word-Level (Subword Features)

**What it is:**
- Each word encoded with rich features
- Character n-grams (handles typos)
- Grammar units
- Phonetic encoding
- Still no word order

**How it works:**
```
"book" → [word_emb + char_ngrams("boo","ook","book") + grammar + phonetic]
"flight" → [word_emb + char_ngrams("fli","lig","igh","ght") + grammar + phonetic]
→ average → classify
```

**When to use:**
- Need typo tolerance
- OOV words common
- User input has errors

**Command:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 30 -epoch 15 -minn 3 -maxn 6
```

### Sentence-Level (Attention)

**What it is:**
- Word order matters
- Self-attention mechanism
- Position encoding
- Compositional meaning
- All subword features included

**How it works:**
```
"book a flight"
→ encode each word with subword features
→ add position encoding (word 0, 1, 2)
→ apply self-attention (Q/K/V)
→ pool to sentence vector
→ classify
```

**When to use:**
- Word order affects meaning
- Complex compositional intents
- Multi-word expressions
- Negation and modifiers

**Command:**
```bash
./gladtotext supervised -input train.txt -output model \
  -dim 50 -epoch 20 -minn 3 -maxn 6 -sentence
```

## Key Test Results

### Test 1: Normal Order
**Input:** "book a flight to paris"

| Approach | Correct? |
|----------|----------|
| Text | ✅ Yes |
| Word | ✅ Yes |
| Sentence | ✅ Yes |

All approaches work for normal input.

### Test 2: Reversed Order
**Input:** "flight book to paris"

| Approach | Correct? | Why |
|----------|----------|-----|
| Text | ⚠️ Confused | No word order |
| Word | ⚠️ Confused | No word order |
| Sentence | ✅ Correct | Attention captures order |

Sentence-level handles unnatural word order.

### Test 3: Typos
**Input:** "bok a flite to paris"

| Approach | Correct? | Why |
|----------|----------|-----|
| Text | ❌ Poor | No subword features |
| Word | ✅ Good | Char n-grams help |
| Sentence | ✅ Best | Subword + context |

Word-level and sentence-level handle typos.

## Comparison Matrix

| Feature | Text | Word | Sentence |
|---------|------|------|----------|
| **Speed** | ⚡⚡⚡ Fastest | ⚡⚡ Fast | ⚡ Slower |
| **Size** | 📦 Smallest | 📦📦 Medium | 📦📦📦 Larger |
| **Word Order** | ❌ No | ❌ No | ✅ Yes |
| **Typo Handling** | ❌ Poor | ✅ Good | ✅ Good |
| **OOV Handling** | ❌ Poor | ✅ Good | ✅ Good |
| **Training Data** | 📊 Minimal | 📊 Moderate | 📊 More |
| **Accuracy (Simple)** | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ |
| **Accuracy (Complex)** | ⭐ | ⭐⭐ | ⭐⭐⭐⭐ |

## Decision Tree

```
Do you need word order awareness?
├─ NO → Do you need typo tolerance?
│       ├─ NO → Use TEXT-LEVEL (fastest)
│       └─ YES → Use WORD-LEVEL (typo-tolerant)
└─ YES → Use SENTENCE-LEVEL (order-aware)
```

## Real-World Use Cases

### Use Case 1: Simple Chatbot
**Task:** Greetings, farewells, basic questions  
**Recommendation:** Text-level or Word-level  
**Why:** Simple intents, speed matters

### Use Case 2: Flight Booking
**Task:** Book, cancel, check status  
**Recommendation:** Sentence-level  
**Why:** Word order matters ("book flight" vs "flight book")

### Use Case 3: Sentiment Analysis
**Task:** Positive/negative reviews  
**Recommendation:** Sentence-level  
**Why:** Negation matters ("not good" vs "good")

### Use Case 4: Spam Detection
**Task:** Spam vs not spam  
**Recommendation:** Word-level  
**Why:** Keywords + typo tolerance, speed important

## How to Run the Example

### Step 1: Run the Script
```bash
./example_intent_classification.sh
```

This will:
1. Create training data
2. Train 3 models
3. Test all models
4. Show comparisons
5. Provide recommendations

### Step 2: Try Your Own Data

```bash
# Create your training data
cat > my_intents.txt << EOF
__intent1 example text for intent 1
__intent2 example text for intent 2
EOF

# Train text-level
./gladtotext supervised -input my_intents.txt -output my_model \
  -dim 30 -epoch 20 -minCount 1

# Train sentence-level
./gladtotext supervised -input my_intents.txt -output my_model_sent \
  -dim 50 -epoch 20 -minCount 1 -sentence

# Test
echo "your test text" | ./gladtotext-infer predict my_model.bin 1
echo "your test text" | ./gladtotext-infer predict my_model_sent.bin 1
```

### Step 3: Compare Results

```bash
# Create test cases
cat > test_cases.txt << EOF
normal word order
reversed order words
text with typos
EOF

# Test both models
echo "Testing text-level:"
cat test_cases.txt | while read line; do
  echo "$line" | ./gladtotext-infer predict my_model.bin 1
done

echo "Testing sentence-level:"
cat test_cases.txt | while read line; do
  echo "$line" | ./gladtotext-infer predict my_model_sent.bin 1
done
```

## Key Takeaways

1. **Three approaches available:**
   - Text-level: Fast, simple, keyword-based
   - Word-level: Typo-tolerant, OOV-robust
   - Sentence-level: Order-aware, context-sensitive

2. **Choose based on task:**
   - Simple tasks → Text-level
   - Typos common → Word-level
   - Order matters → Sentence-level

3. **All approaches work together:**
   - Can train multiple models
   - Compare results
   - Choose best for your use case

4. **Easy to use:**
   - Single command-line flag (`-sentence`)
   - Automatic detection during inference
   - No code changes needed

## Next Steps

1. **Run the example:**
   ```bash
   ./example_intent_classification.sh
   ```

2. **Read the documentation:**
   - [INTENT_CLASSIFICATION_EXAMPLE.md](INTENT_CLASSIFICATION_EXAMPLE.md)
   - [SENTENCE_ENCODING.md](SENTENCE_ENCODING.md)
   - [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

3. **Try with your data:**
   - Create training file
   - Train models
   - Compare results

4. **Optimize:**
   - Tune hyperparameters
   - Add more training data
   - Choose best approach

## Summary

The complete example demonstrates:
- ✅ Three approaches to intent classification
- ✅ Working code and scripts
- ✅ Comprehensive documentation
- ✅ Real test results
- ✅ Clear recommendations
- ✅ Easy to run and modify

Everything is ready to use for production intent classification!
