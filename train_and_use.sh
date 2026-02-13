#!/bin/bash

# Step 1: Create training data
cat > train_chatbot.txt << 'EOF'
__greeting hello
__greeting hi there
__greeting hey
__greeting good morning
__greeting what's up
__farewell goodbye
__farewell bye
__farewell see you later
__farewell take care
__question how are you
__question what's your name
__question can you help me
EOF

# Step 2: Train model using C++ CLI
echo "Training model..."
./gladtotext supervised -input train_chatbot.txt -output chatbot -dim 50 -epoch 20 -lr 0.1

# Step 3: Use in Python
echo ""
echo "Testing Python API..."
python3 << 'PYTHON'
import gladtotext

# Load model
model = gladtotext.Model()
if not model.load_model("chatbot.bin"):
    print("Failed to load model")
    exit(1)

# Get model info
info = model.get_info()
print(f"Model loaded: {info['num_labels']} labels, dim={info['dim']}")
print(f"Labels: {info['labels']}")
print()

# Test predictions
test_texts = [
    "hello friend",
    "goodbye everyone", 
    "how are you doing",
    "hey there"
]

for text in test_texts:
    predictions = model.predict(text, k=2)
    print(f"Text: '{text}'")
    for label, score in predictions:
        print(f"  {label}: {score:.4f}")
    print()
PYTHON

# Cleanup
rm -f train_chatbot.txt chatbot.bin
