#!/bin/bash

# Master test runner for comprehensive test suite

echo "=========================================="
echo "GLADtoTEXT Comprehensive Test Suite"
echo "=========================================="
echo ""

# Track results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name=$1
    local test_script=$2
    
    echo "Running: $test_name"
    echo "----------------------------------------"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if bash $test_script; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo "✓ $test_name PASSED"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo "✗ $test_name FAILED"
        return 1
    fi
    
    echo ""
}

# Change to test directory
cd "$(dirname "$0")"

# Run all comprehensive tests
run_test "Sentiment Classification (3 classes)" "test_sentiment_classification.sh"
run_test "Intent Classification (8 classes)" "test_intent_classification.sh"
run_test "Topic Classification (8 classes)" "test_topic_classification.sh"

# Run performance benchmarks
echo "=========================================="
echo "Performance Benchmarks"
echo "=========================================="
echo ""

TRAIN_DATA="../data/sentiment_large.txt"

echo "Benchmark 1: Baseline (batch=1, full attention)"
time ../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output bench_baseline \
  -dim 100 \
  -epoch 10 \
  -lr 0.1 \
  -minCount 1 \
  > /dev/null 2>&1
echo ""

echo "Benchmark 2: Batch processing (batch=32)"
time ../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output bench_batch \
  -dim 100 \
  -epoch 10 \
  -lr 0.1 \
  -minCount 1 \
  -batch 32 \
  > /dev/null 2>&1
echo ""

echo "Benchmark 3: Optimized (batch=32, simple attention)"
time ../../gladtotext supervised \
  -input $TRAIN_DATA \
  -output bench_optimized \
  -dim 100 \
  -epoch 10 \
  -lr 0.1 \
  -minCount 1 \
  -batch 32 \
  -attention simple \
  > /dev/null 2>&1
echo ""

# Cleanup benchmark files
rm -f bench_*.bin

# Print summary
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Total tests: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo "✓ ALL TESTS PASSED!"
    echo "=========================================="
    exit 0
else
    echo "✗ SOME TESTS FAILED"
    echo "=========================================="
    exit 1
fi
