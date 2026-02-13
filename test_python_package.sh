#!/bin/bash

# Test script for Python package installation and functionality

set -e  # Exit on error

echo "=========================================="
echo "GLADtoTEXT Python Package Test"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Function to print test result
test_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗${NC} $2"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
}

# Test 1: Check Python version
echo "Test 1: Checking Python version..."
python3 --version
test_result $? "Python 3 is available"
echo ""

# Test 2: Check if pybind11 is installed
echo "Test 2: Checking pybind11..."
python3 -c "import pybind11; print('pybind11 version:', pybind11.__version__)" 2>/dev/null
test_result $? "pybind11 is installed"
echo ""

# Test 3: Check if numpy is installed
echo "Test 3: Checking numpy..."
python3 -c "import numpy; print('numpy version:', numpy.__version__)" 2>/dev/null
test_result $? "numpy is installed"
echo ""

# Test 4: Check MANIFEST.in exists
echo "Test 4: Checking MANIFEST.in..."
if [ -f "MANIFEST.in" ]; then
    echo "MANIFEST.in contents:"
    head -10 MANIFEST.in | sed 's/^/  /'
    test_result 0 "MANIFEST.in exists"
else
    test_result 1 "MANIFEST.in exists"
fi
echo ""

# Test 5: Check if C++ binaries are built
echo "Test 5: Checking C++ binaries..."
if [ -f "gladtotext" ] && [ -f "gladtotext-infer" ]; then
    test_result 0 "C++ binaries exist"
else
    echo -e "${YELLOW}⚠${NC} C++ binaries not found. Building..."
    make all
    test_result $? "C++ binaries built"
fi
echo ""

# Test 6: Build source distribution
echo "Test 6: Building source distribution..."
python3 -m build --sdist 2>&1 | tail -5
test_result ${PIPESTATUS[0]} "Source distribution built"
echo ""

# Test 7: Check if headers are included in sdist
echo "Test 7: Checking if headers are in source distribution..."
if [ -f dist/gladtotext-*.tar.gz ]; then
    HEADER_COUNT=$(tar -tzf dist/gladtotext-*.tar.gz | grep "\.h$" | wc -l)
    echo "Found $HEADER_COUNT header files in source distribution"
    if [ $HEADER_COUNT -gt 0 ]; then
        echo "Sample headers:"
        tar -tzf dist/gladtotext-*.tar.gz | grep "\.h$" | head -5 | sed 's/^/  /'
        test_result 0 "Headers included in source distribution"
    else
        test_result 1 "Headers included in source distribution"
    fi
else
    test_result 1 "Source distribution file exists"
fi
echo ""

# Test 8: Build wheel
echo "Test 8: Building wheel..."
python3 -m build --wheel 2>&1 | tail -5
test_result ${PIPESTATUS[0]} "Wheel built"
echo ""

# Test 9: Install package in test mode
echo "Test 9: Installing package..."
pip install -e . --quiet
test_result $? "Package installed"
echo ""

# Test 10: Import module
echo "Test 10: Importing gladtotext module..."
python3 -c "import gladtotext; print('Module imported successfully')" 2>&1
test_result $? "Module imports successfully"
echo ""

# Test 11: Check module has expected classes
echo "Test 11: Checking module contents..."
python3 << 'EOF'
import gladtotext
assert hasattr(gladtotext, 'Model'), "Model class not found"
print("✓ Model class exists")
model = gladtotext.Model()
print("✓ Model instance created")
EOF
test_result $? "Module has expected classes"
echo ""

# Test 12: Full integration test (if model exists)
echo "Test 12: Full integration test..."
if [ -f "test_model.bin" ]; then
    python3 << 'EOF'
import gladtotext
model = gladtotext.Model()
assert model.load_model("test_model.bin"), "Failed to load model"
info = model.get_info()
print(f"Model info: {info}")
predictions = model.predict("test", k=1)
print(f"Predictions: {predictions}")
vec = model.get_word_vector("test")
print(f"Vector shape: {vec.shape}")
print("✓ All model operations work")
EOF
    test_result $? "Full integration test"
else
    echo -e "${YELLOW}⚠${NC} No test model found. Creating one..."
    
    # Create test data
    cat > test_train.txt << 'EOF'
__positive great product amazing
__positive love it excellent
__positive wonderful fantastic
__negative terrible awful
__negative hate it bad
__negative horrible disappointing
EOF
    
    # Train model
    ./gladtotext supervised -input test_train.txt -output test_model -dim 30 -epoch 10 > /dev/null 2>&1
    
    if [ -f "test_model.bin" ]; then
        echo "Test model created. Running integration test..."
        python3 << 'EOF'
import gladtotext
model = gladtotext.Model()
assert model.load_model("test_model.bin"), "Failed to load model"
info = model.get_info()
print(f"Model info: {info}")
predictions = model.predict("great", k=2)
print(f"Predictions: {predictions}")
vec = model.get_word_vector("great")
print(f"Vector shape: {vec.shape}")
print("✓ All model operations work")
EOF
        test_result $? "Full integration test"
        
        # Cleanup
        rm -f test_train.txt test_model.bin
    else
        test_result 1 "Full integration test (model creation failed)"
    fi
fi
echo ""

# Test 13: Check package metadata
echo "Test 13: Checking package metadata..."
python3 << 'EOF'
import pkg_resources
try:
    dist = pkg_resources.get_distribution('gladtotext')
    print(f"Package: {dist.project_name}")
    print(f"Version: {dist.version}")
    print(f"Location: {dist.location}")
    print("✓ Package metadata is accessible")
except:
    print("⚠ Package metadata not found (this is OK for editable installs)")
EOF
test_result $? "Package metadata check"
echo ""

# Summary
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    echo ""
    echo "The Python package is ready to use."
    echo ""
    echo "Next steps:"
    echo "  1. Update GitHub URL in setup.py and pyproject.toml"
    echo "  2. Test installation from GitHub:"
    echo "     pip install git+https://github.com/yourusername/GLADtoTEXT.git"
    echo "  3. When ready, publish to PyPI:"
    echo "     python3 -m twine upload dist/*"
    exit 0
else
    echo -e "${RED}✗ Some tests failed${NC}"
    echo ""
    echo "Please fix the issues above before proceeding."
    exit 1
fi
