#!/bin/bash

# Master stress test runner

echo "=========================================="
echo "GLADtoTEXT Stress Test Suite"
echo "=========================================="
echo ""
echo "This comprehensive test suite includes:"
echo "  • Large-scale classification (10K examples, 12 classes)"
echo "  • Edge case testing (typos, negations, special cases)"
echo "  • Stress testing (memory, concurrency, large batches)"
echo "  • Robustness testing (variations, tolerance)"
echo ""
echo "Expected runtime: 10-20 minutes"
echo ""

# Track results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name=$1
    local test_script=$2
    
    echo "=========================================="
    echo "Running: $test_name"
    echo "=========================================="
    echo ""
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if bash $test_script; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo ""
        echo "✓ $test_name PASSED"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo ""
        echo "✗ $test_name FAILED"
        return 1
    fi
    
    echo ""
}

# Change to test directory
cd "$(dirname "$0")"

# Check if binaries exist
if [ ! -f "../../gladtotext" ] || [ ! -f "../../gladtotext-infer" ]; then
    echo "Error: gladtotext binaries not found!"
    echo "Please run 'make' in the root directory first."
    exit 1
fi

# Check if data files exist
if [ ! -f "../data/large_scale_train.txt" ]; then
    echo "Error: Test data files not found!"
    echo "Please ensure tests/data/large_scale_train.txt exists."
    exit 1
fi

# Run all stress tests
START_TIME=$(date +%s)

run_test "Large-Scale Classification (10K examples, 12 classes)" "test_large_scale.sh"
run_test "Edge Case Testing (typos, negations, special cases)" "test_edge_cases.sh"
run_test "Stress Testing (memory, concurrency, large batches)" "test_stress.sh"
run_test "Robustness Testing (variations, tolerance)" "test_robustness.sh"

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))
MINUTES=$((DURATION / 60))
SECONDS=$((DURATION % 60))

# Print summary
echo "=========================================="
echo "Stress Test Summary"
echo "=========================================="
echo "Total test suites: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"
echo "Total runtime: ${MINUTES}m ${SECONDS}s"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo "✓ ALL STRESS TESTS PASSED!"
    echo ""
    echo "The system has been thoroughly tested and is ready for:"
    echo "  • Production deployment"
    echo "  • Large-scale datasets (10K+ examples)"
    echo "  • Multiple classes (12+ categories)"
    echo "  • Edge cases and variations"
    echo "  • High-load scenarios"
    echo "  • Concurrent usage"
    echo ""
    echo "=========================================="
    exit 0
else
    echo "✗ SOME STRESS TESTS FAILED"
    echo ""
    echo "Please review the failed tests above."
    echo "=========================================="
    exit 1
fi
