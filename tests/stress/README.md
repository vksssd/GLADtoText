# Stress Test Suite

Comprehensive stress testing for GLADtoTEXT with large-scale datasets, edge cases, and production scenarios.

## Overview

This test suite validates the system's behavior under real-world conditions with:
- 10,000+ training examples
- 12 classification categories
- Edge cases (typos, negations, special characters)
- Stress scenarios (memory, concurrency, large batches)
- Robustness testing (variations, tolerance)

## Test Suites

### 1. Large-Scale Classification (`test_large_scale.sh`)

Tests the system with a realistic 10K example dataset across 12 categories:
- customer_support
- product_inquiry
- order_status
- payment_issue
- return_refund
- account_management
- technical_issue
- shipping_delivery
- product_complaint
- feedback_review
- pricing_discount
- general_inquiry

**Tests:**
- Basic training on 10K examples
- Training with validation set (1.2K examples)
- Batch processing (batch=64)
- Optimized configuration (batch + simple attention)
- Inference for all 12 categories
- Sentence encoding mode
- Model size verification

**Expected runtime:** 3-5 minutes

### 2. Edge Case Testing (`test_edge_cases.sh`)

Tests robustness with 114 edge cases including:
- Heavy typos (80% typo rate)
- Multiple negations
- Very short sentences (2 words)
- Very long sentences (20+ words)
- Case variations (ALL CAPS, lowercase, MiXeD)
- Special characters and numbers
- Word repetition
- Batch inference on all edge cases

**Tests:**
- Typo tolerance
- Negation handling
- Minimal context (2-word sentences)
- Extended sentences (20+ words)
- Case sensitivity
- Special characters
- Repeated words
- Batch processing of edge cases
- Sentence encoding with edge cases

**Expected runtime:** 2-3 minutes

### 3. Stress Testing (`test_stress.sh`)

Tests system limits and performance under load:

**Tests:**
- Large dimensions (300, 500)
- Large batch sizes (64, 128, 256)
- Extended training (100 epochs with early stopping)
- Concurrent inference (10 parallel processes)
- Rapid sequential inference (1000 predictions)
- Batch inference on large dataset
- Memory stress with sentence encoding
- All features combined
- Model file integrity (5 save/load cycles)

**Expected runtime:** 4-6 minutes

### 4. Robustness Testing (`test_robustness.sh`)

Tests model behavior with various input variations:

**Tests:**
- Typo tolerance levels (light to heavy)
- Negation handling (single to complex)
- Sentence length variations (2 to 26+ words)
- Word order robustness
- Bag-of-words vs sentence encoding comparison
- Special character handling
- Case sensitivity
- Repeated word handling
- Pronoun variations

**Expected runtime:** 2-3 minutes

## Running Tests

### Run All Stress Tests

```bash
cd tests/stress
bash run_stress_tests.sh
```

Or from the root directory:

```bash
make test-stress
```

### Run Individual Test Suites

```bash
cd tests/stress

# Large-scale classification
bash test_large_scale.sh

# Edge cases
bash test_edge_cases.sh

# Stress testing
bash test_stress.sh

# Robustness testing
bash test_robustness.sh
```

## Test Data

### Large-Scale Training Data
- **File:** `tests/data/large_scale_train.txt`
- **Size:** 10,000 examples
- **Classes:** 12 categories
- **Variations:** Typos, negations, pronouns, intensifiers, fillers, word jumbling, length variations

### Large-Scale Validation Data
- **File:** `tests/data/large_scale_validation.txt`
- **Size:** 1,200 examples (100 per category)
- **Purpose:** Validation set for early stopping and accuracy measurement

### Edge Cases
- **File:** `tests/data/edge_cases.txt`
- **Size:** 114 examples
- **Coverage:**
  - Heavy typos (80% typo rate)
  - Multiple negations (2-4 negations per sentence)
  - Very short sentences (2 words)
  - Very long sentences (3x extended with fillers)
  - All caps, all lowercase, mixed case
  - Repeated words
  - Numbers and special characters
  - Minimal examples

## Expected Results

### Performance Benchmarks

With the optimized configuration (batch=64, simple attention):
- Training speed: ~2-3 minutes for 10K examples (20 epochs)
- Inference speed: ~100-200 predictions/second
- Model size: ~5-15 MB (depending on dimensions)

### Accuracy Expectations

- Standard cases: 85-95% accuracy
- Light typos: 80-90% accuracy
- Medium typos: 70-85% accuracy
- Heavy typos: 60-75% accuracy
- Edge cases: 50-70% accuracy (expected degradation)

### Stress Test Limits

Successfully tested with:
- Dimensions: up to 500
- Batch sizes: up to 256
- Concurrent processes: 10 parallel
- Sequential predictions: 1000+ rapid-fire
- Training examples: 10,000+
- Classes: 12+

## Interpreting Results

### Success Criteria

All tests should pass (✓) with:
- No crashes or segmentation faults
- Reasonable accuracy on standard cases
- Graceful degradation on edge cases
- Stable performance under load

### Warning Signs (⚠)

Some edge cases may show warnings, which is expected:
- Heavy typos may reduce accuracy
- Complex negations may confuse the model
- Very short sentences may lack context
- Word order shuffling may affect sentence encoding

These warnings indicate expected behavior, not failures.

### Failure Indicators (✗)

Tests fail if:
- Training crashes or hangs
- Inference produces no output
- Model files are corrupted
- Memory errors occur
- Concurrent processes crash

## Production Readiness

After passing all stress tests, the system is validated for:

✓ Large-scale datasets (10K+ examples)
✓ Multiple classes (12+ categories)
✓ Real-world variations (typos, negations, etc.)
✓ High-load scenarios (concurrent usage)
✓ Extended training (100+ epochs)
✓ Large models (500+ dimensions)
✓ Batch processing (256+ batch size)
✓ Edge case handling
✓ Model persistence and integrity

## Troubleshooting

### Tests Fail to Run

1. Ensure binaries are compiled:
   ```bash
   make
   ```

2. Check data files exist:
   ```bash
   ls tests/data/large_scale_train.txt
   ls tests/data/edge_cases.txt
   ```

3. Make scripts executable:
   ```bash
   chmod +x tests/stress/*.sh
   ```

### Memory Issues

If tests fail with memory errors:
- Reduce dimensions (try 100 instead of 500)
- Reduce batch size (try 32 instead of 256)
- Close other applications

### Slow Performance

If tests take too long:
- Use optimized configuration (batch + simple attention)
- Reduce epochs for testing
- Skip concurrent tests if needed

## Next Steps

After passing stress tests:

1. Review `PRODUCTION_READINESS_STATUS.md` for deployment checklist
2. Run integration tests: `make test-integration`
3. Run comprehensive tests: `make test-comprehensive`
4. Deploy to production environment
5. Monitor performance metrics
6. Set up continuous testing

## Contributing

When adding new stress tests:

1. Follow the existing test structure
2. Include clear test descriptions
3. Add both positive and negative cases
4. Document expected behavior
5. Update this README
6. Add to `run_stress_tests.sh`
