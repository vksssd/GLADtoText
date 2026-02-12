# Contributing to GLADtoTEXT

## Project Structure

```
GLADtoTEXT/
├── core/              # Core components (vector, matrix, dictionary, config)
├── model/             # Model components (embeddings, attention, backbone)
├── subwords/          # Subword features (char n-grams, grammar, phonetic)
├── heads/             # Task heads (classification, semantic)
├── training/          # Training implementations
├── tests/             # Test suite
│   ├── unit/         # Unit tests
│   └── integration/  # Integration tests
├── examples/          # Example scripts
├── README.md          # Project overview
├── GUIDE.md           # Complete guide
└── CONTRIBUTING.md    # This file
```

## Development Workflow

### 1. Setup

```bash
git clone https://github.com/yourusername/GLADtoTEXT.git
cd GLADtoTEXT
make all
make test
```

### 2. Make Changes

```bash
# Edit source files
vim core/vector.h

# Build
make clean && make all

# Test
make test
```

### 3. Add Tests

```bash
# Add unit test
vim tests/unit/test_myfeature.cpp

# Add integration test
vim tests/integration/test_myfeature.sh
chmod +x tests/integration/test_myfeature.sh
```

### 4. Update Documentation

```bash
# Update guide
vim GUIDE.md

# Update README if needed
vim README.md
```

## Code Style

### C++ Guidelines

- Use C++17 features
- Follow existing naming conventions
- Add comments for complex logic
- Include error handling
- Use const correctness

### Example

```cpp
// Good
class MyClass {
public:
    explicit MyClass(int size) : data(size) {
        if (size <= 0) {
            throw std::invalid_argument("Size must be positive");
        }
    }
    
    Vector compute(const Vector& input) const {
        if (input.size() != data.size()) {
            return Vector(data.size());  // Return zero vector
        }
        // ... computation
    }
    
private:
    std::vector<float> data;
};
```

## Testing

### Unit Tests

Test individual components:

```cpp
#include <iostream>
#include "../../core/vector.h"

int main() {
    Vector v(10);
    v.v[0] = 1.0f;
    
    if (v.v[0] == 1.0f) {
        std::cout << "PASS\n";
        return 0;
    }
    std::cout << "FAIL\n";
    return 1;
}
```

### Integration Tests

Test complete workflows:

```bash
#!/bin/bash
echo "=== My Test ==="

# Create data
cat > test.txt << EOF
__label1 example
EOF

# Train
../../gladtotext supervised -input test.txt -output model \
  -dim 20 -epoch 5 -minCount 1 > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ Test passed"
else
    echo "✗ Test failed"
    exit 1
fi

# Cleanup
rm -f test.txt model.bin
```

## Adding Features

### 1. Core Component

```cpp
// core/myfeature.h
#pragma once
#include "vector.h"

class MyFeature {
public:
    explicit MyFeature(int dim) : dimension(dim) {}
    
    Vector process(const Vector& input) const {
        // Implementation
    }
    
private:
    int dimension;
};
```

### 2. Add Tests

```cpp
// tests/unit/test_myfeature.cpp
#include "../../core/myfeature.h"

int main() {
    MyFeature feature(10);
    Vector input(10);
    Vector output = feature.process(input);
    
    std::cout << "PASS\n";
    return 0;
}
```

### 3. Update Makefile

```makefile
tests/unit/t6: tests/unit/test_myfeature.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@
```

### 4. Document

Add to GUIDE.md under appropriate section.

## Performance Optimization

### Guidelines

1. **Numerical Stability**
   - Use log-sum-exp trick for softmax
   - Clip gradients to prevent explosion
   - Handle edge cases (empty input, zero division)

2. **Memory Efficiency**
   - Use sparse matrices where appropriate
   - Implement pruning for small weights
   - Lazy initialization

3. **Computation Efficiency**
   - Vectorize operations
   - Reduce redundant computations
   - Cache frequently accessed data

### Example: Stable Softmax

```cpp
// Bad
float denom = 0.0f;
for (float x : scores) {
    denom += std::exp(x);
}

// Good
float maxScore = *std::max_element(scores.begin(), scores.end());
float denom = 0.0f;
for (float s : scores) {
    denom += std::exp(s - maxScore);
}
```

## Common Issues

### Compiler Warnings

Fix all warnings before committing:

```bash
make clean && make all 2>&1 | grep warning
```

### Memory Leaks

Use proper RAII and smart pointers:

```cpp
// Bad
MyClass* obj = new MyClass();
// ... might leak

// Good
std::unique_ptr<MyClass> obj = std::make_unique<MyClass>();
// Automatically cleaned up
```

### Numerical Issues

Add bounds checking and clipping:

```cpp
// Prevent overflow
score = std::max(-10.0f, std::min(10.0f, score));

// Prevent division by zero
if (std::abs(denom) < 1e-8f) {
    return 0.0f;
}
```

## Documentation

### Code Comments

```cpp
/**
 * Compute attention-weighted representation
 * 
 * @param embeddings Input word embeddings
 * @return Weighted sum of embeddings
 */
Vector computeAttention(const std::vector<Vector>& embeddings) const;
```

### Guide Updates

When adding features, update GUIDE.md:

1. Add to relevant section
2. Include example usage
3. Explain when to use
4. Add to quick reference

## Pull Request Process

1. **Create branch**
   ```bash
   git checkout -b feature/my-feature
   ```

2. **Make changes**
   - Write code
   - Add tests
   - Update docs

3. **Test thoroughly**
   ```bash
   make clean
   make all
   make test
   ```

4. **Commit**
   ```bash
   git add .
   git commit -m "Add my feature"
   ```

5. **Push and create PR**
   ```bash
   git push origin feature/my-feature
   ```

## Areas for Contribution

### High Priority

- [ ] Multi-head attention
- [ ] Layer normalization
- [ ] Beam search for generation
- [ ] GPU acceleration
- [ ] Model compression

### Medium Priority

- [ ] Additional subword features
- [ ] More attention mechanisms
- [ ] Quantization schemes
- [ ] Language-specific optimizations

### Low Priority

- [ ] Additional examples
- [ ] Documentation improvements
- [ ] Performance benchmarks
- [ ] Visualization tools

## Questions?

- Check [GUIDE.md](GUIDE.md) for usage
- Check [tests/](tests/) for examples
- Open an issue for bugs
- Start a discussion for features

## Code of Conduct

- Be respectful
- Be constructive
- Focus on the code, not the person
- Help others learn

---

Thank you for contributing to GLADtoTEXT!
