#!/bin/bash

# Demo: Sentence-level encoding for intent classification
# This shows the difference between bag-of-words and sentence-level encoding

echo "=== GLADtoTEXT Sentence Encoding Demo ==="
echo ""

# Create sample training data
cat > train_intent.txt << 'EOF'
__greeting hello
__greeting hi there
__greeting good morning
__greeting hey how are you
__farewell goodbye
__farewell see you later
__farewell bye bye
__farewell take care
__question what is your name
__question how does this work
__question can you help me
__question where is the station
EOF

echo "Training data created (train_intent.txt)"
echo ""

# Train WITHOUT sentence encoding (bag-of-words)
echo "1. Training WITHOUT sentence encoding (bag-of-words)..."
make gladtotext
./gladtotext supervised -input train_intent.txt -output model_bow -dim 50 -epoch 10 -lr 0.1
echo ""

# Train WITH sentence encoding
echo "2. Training WITH sentence encoding..."
./gladtotext supervised -input train_intent.txt -output model_sentence -dim 50 -epoch 10 -lr 0.1 -sentence
echo ""

# Create test data
cat > test_intent.txt << 'EOF'
hello friend
good morning everyone
bye for now
what time is it
EOF

echo "Test data created (test_intent.txt)"
echo ""

# Test both models
echo "3. Testing bag-of-words model:"
make gladtotext-infer
./gladtotext-infer predict model_bow.bin 1 < test_intent.txt
echo ""

echo "4. Testing sentence encoding model:"
./gladtotext-infer predict model_sentence.bin 1 < test_intent.txt
echo ""

echo "=== Demo Complete ==="
echo ""
echo "Key differences:"
echo "- Bag-of-words: Treats text as unordered set of words"
echo "- Sentence encoding: Considers word order and context using self-attention"
echo ""
echo "The sentence encoder is especially useful for:"
echo "- Intent classification where word order matters"
echo "- Handling out-of-vocabulary words with subword features"
echo "- Capturing compositional meaning in longer sentences"
