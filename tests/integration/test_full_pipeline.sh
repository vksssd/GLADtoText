#!/bin/bash

# Integration test: Full training and inference pipeline

echo "=== Full Pipeline Integration Test ==="
echo ""

# Create test data
cat > test_data.txt << 'EOF'
__positive great movie
__positive excellent film
__positive amazing show
__negative bad movie
__negative terrible film
__negative awful show
EOF

echo "1. Training model..."
../../gladtotext supervised -input test_data.txt -output test_model \
  -dim 20 -epoch 10 -lr 0.1 -minCount 1 > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Training successful"
else
    echo "✗ Training failed"
    exit 1
fi

echo ""
echo "2. Testing inference..."
result=$(echo "great film" | ../../gladtotext-infer predict test_model.bin 1 2>/dev/null | head -1)

if [[ $result == *"__positive"* ]]; then
    echo "✓ Inference successful: $result"
else
    echo "✗ Inference failed: $result"
    exit 1
fi

echo ""
echo "3. Checking model file..."
if [ -f "test_model.bin" ]; then
    size=$(ls -lh test_model.bin | awk '{print $5}')
    echo "✓ Model file created: $size"
else
    echo "✗ Model file not found"
    exit 1
fi

# Cleanup
rm -f test_data.txt test_model.bin

echo ""
echo "=== All integration tests passed! ==="
