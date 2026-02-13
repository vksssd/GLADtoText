#!/bin/bash
# Test script for Python bindings

set -e

echo "=========================================="
echo "Testing GLADtoTEXT Python Bindings"
echo "=========================================="
echo ""

# Check if C++ binaries exist
if [ ! -f "./gladtotext" ]; then
    echo "Building C++ binaries..."
    make all
fi

# Create test data
echo "Creating test data..."
cat > test_train.txt << EOF
__greeting hello
__greeting hi there
__greeting good morning
__greeting hey
__farewell goodbye
__farewell see you later
__farewell bye
__farewell take care
__question what is this
__question how does it work
__question can you help
__question why
EOF

# Train a small model
echo "Training test model..."
./gladtotext supervised -input test_train.txt -output test_model \
  -dim 20 -epoch 10 -minCount 1

# Check if Python package is installed
echo ""
echo "Checking Python installation..."
if ! python3 -c "import gladtotext" 2>/dev/null; then
    echo "Python package not installed. Installing..."
    pip install -e .
fi

# Test Python bindings
echo ""
echo "Testing Python bindings..."
python3 << 'EOF'
import gladtotext
import sys

print("✓ Import successful")

# Load model
model = gladtotext.Model()
if not model.load_model("test_model.bin"):
    print("✗ Failed to load model")
    sys.exit(1)
print("✓ Model loaded")

# Get info
info = model.get_info()
print(f"✓ Model info: {info['vocab_size']} words, dim={info['dim']}")

# Get word vector
try:
    vec = model.get_word_vector("hello")
    print(f"✓ Word vector shape: {vec.shape}")
except Exception as e:
    print(f"✗ Failed to get word vector: {e}")
    sys.exit(1)

# Predict
try:
    predictions = model.predict("hello friend", k=2)
    print(f"✓ Predictions: {predictions}")
    
    # Check prediction format
    if len(predictions) > 0:
        label, score = predictions[0]
        print(f"  Top prediction: {label} ({score:.4f})")
except Exception as e:
    print(f"✗ Failed to predict: {e}")
    sys.exit(1)

print("\n✓ All tests passed!")
EOF

# Cleanup
echo ""
echo "Cleaning up..."
rm -f test_train.txt test_model.bin

echo ""
echo "=========================================="
echo "✓ Python bindings working correctly!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "  1. Read PYTHON_GUIDE.md for detailed usage"
echo "  2. Try python/example_usage.py"
echo "  3. Train your own models!"
