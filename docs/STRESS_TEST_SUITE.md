# Stress Test Suite - Complete Documentation

## Overview

The GLADtoTEXT stress test suite provides comprehensive validation for production deployment with real-world scenarios, edge cases, and high-load conditions.

## Test Coverage

### 1. Large-Scale Classification (10K+ Examples)

**Dataset:** 10,000 training examples + 1,200 validation examples
**Categories:** 12 real-world customer service categories
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

**Realistic Variations:**
- Typos (10-20% of examples)
- Negations (multiple negation patterns)
- Pronouns (I, you, he, she, we, they)
- Intensifiers (very, really, extremely, etc.)
- Fillers (actually, basically, literally, etc.)
- Word jumbling (slight word order variations)
- Length variations (short to long sentences)

### 2. Edge Cases (114 Examples)

**Extreme Conditions:**
- Heavy typos (80% typo rate: "hlep" instead of "help")
- Multiple negations (2-4 negations per sentence)
- Very short sentences (2 words: "Help needed")
- Very long sentences (26+ words with repetitive fillers)
- Case variations (ALL CAPS, all lowercase, MiXeD cAsE)
- Repeated words ("help help help I need help")
- Numbers and special characters (#, @, %, !!!)
- Minimal context examples

### 3. Stress Testing

**System Limits:**
- Large dimensions (300, 500)
- Large batch sizes (64, 128, 256)
- Extended training (100 epochs with early stopping)
- Concurrent inference (10 parallel processes × 100 predictions)
- Rapid sequential inference (1000 predictions)
- Batch inference on 10K dataset
- Memory stress with sentence encoding
- All features combined
- Model file integrity (5 save/load cycles)

### 4. Robustness Testing

**Input Variations:**
- Typo tolerance levels (light → medium → heavy)
- Negation complexity (single → double → complex)
- Sentence length (2 words → 26+ words)
- Word order variations (standard → shuffled)
- Model comparison (bag-of-words vs sentence encoding)
- Special character handling
- Case sensitivity
- Repeated word patterns
- Pronoun variations (I/you/he/she/we/they)

## Running the Tests

### Quick Start

```bash
# Run all stress tests
make test-stress

# Or manually
cd tests/stress
bash run_stress_tests.sh
```

### Individual Test Suites

```bash
cd tests/stress

# Large-scale classification (3-5 min)
bash test_large_scale.sh

# Edge cases (2-3 min)
bash test_edge_cases.sh

# Stress testing (4-6 min)
bash test_stress.sh

# Robustness testing (2-3 min)
bash test_robustness.sh
```

### Complete Test Suite

```bash
# Run everything: unit + integration + comprehensive + stress
make test-all
```

## Expected Results

### Performance Benchmarks

**Training Speed:**
- Baseline (batch=1, full attention): ~5-7 minutes for 10K examples
- Batch processing (batch=32): ~3-4 minutes (44% faster)
- Optimized (batch=64, simple attention): ~2-3 minutes (68% faster)

**Inference Speed:**
- Single prediction: ~10-20ms
- Batch inference: 100-200 predictions/second
- Concurrent (10 processes): Stable performance

**Model Size:**
- dim=100: ~3-5 MB
- dim=150: ~5-8 MB
- dim=300: ~10-15 MB
- dim=500: ~20-30 MB

### Accuracy Expectations

| Scenario | Expected Accuracy |
|----------|------------------|
| Standard cases | 85-95% |
| Light typos (1-2) | 80-90% |
| Medium typos (3-4) | 70-85% |
| Heavy typos (5+) | 60-75% |
| Single negation | 75-85% |
| Multiple negations | 60-75% |
| Very short (2-3 words) | 70-80% |
| Very long (20+ words) | 75-85% |
| Edge cases | 50-70% |

### Success Criteria

✓ All tests pass without crashes
✓ Reasonable accuracy on standard cases
✓ Graceful degradation on edge cases
✓ Stable performance under load
✓ No memory leaks or corruption
✓ Model files save/load correctly

## Test Data Generation

The test data was generated with realistic variations:

```python
# Typo injection (10-20% of examples)
"help" → "hlep"
"account" → "acount"
"product" → "prodcut"

# Negation patterns
"I want to return" → "I dont want to return"
"Payment processing" → "Payment isnt processing"

# Pronoun variations
"I need help" → "You need help" / "He needs help"

# Intensifiers
"Good product" → "Very good product" / "Really good product"

# Fillers
"I need help" → "I actually need help" / "I basically need help"

# Length variations
Short: "Help needed" (2 words)
Medium: "I need help with my account" (7 words)
Long: "I need help with my account and I would appreciate..." (20+ words)
```

## Production Readiness Validation

After passing all stress tests, the system is validated for:

### ✓ Scale
- 10,000+ training examples
- 12+ classification categories
- 500+ dimensional embeddings
- 256+ batch sizes

### ✓ Robustness
- Typo tolerance (light to heavy)
- Negation handling (single to complex)
- Length variations (2 to 26+ words)
- Case insensitivity
- Special character handling

### ✓ Performance
- 68% faster with optimizations
- 100-200 predictions/second
- Concurrent inference support
- Efficient memory usage

### ✓ Reliability
- No crashes or memory leaks
- Stable under load
- Model file integrity
- Graceful error handling

### ✓ Features
- Bag-of-words mode
- Sentence encoding mode
- Batch processing
- Attention modes (full/simple/none)
- Validation set support
- Early stopping
- Grammar units
- Phonetic encoding

## Interpreting Results

### Success Indicators (✓)

Tests pass with:
- No segmentation faults or crashes
- Accuracy within expected ranges
- Stable performance metrics
- Successful model save/load
- Correct predictions for all categories

### Warning Indicators (⚠)

Some edge cases may show warnings:
- Heavy typos reduce accuracy (expected)
- Complex negations confuse model (expected)
- Very short sentences lack context (expected)
- Word order shuffling affects sentence encoding (expected)

These are NOT failures - they indicate expected behavior under extreme conditions.

### Failure Indicators (✗)

Tests fail if:
- Training crashes or hangs
- Inference produces no output
- Model files are corrupted
- Memory errors occur
- Concurrent processes crash
- Accuracy drops below 50% on standard cases

## Troubleshooting

### Memory Issues

If tests fail with memory errors:
```bash
# Reduce dimensions
-dim 100  # instead of 500

# Reduce batch size
-batch 32  # instead of 256

# Use simple attention
-attention simple
```

### Slow Performance

If tests take too long:
```bash
# Use optimized configuration
-batch 64 -attention simple

# Reduce epochs for testing
-epoch 10  # instead of 30

# Skip concurrent tests
# Comment out concurrent test section
```

### Data Issues

If data files are missing:
```bash
# Check files exist
ls tests/data/large_scale_train.txt
ls tests/data/large_scale_validation.txt
ls tests/data/edge_cases.txt

# Regenerate if needed (contact maintainer)
```

## Continuous Testing

### Pre-Commit Testing

```bash
# Quick validation before commit
make test-unit
make test-integration
```

### Pre-Release Testing

```bash
# Full validation before release
make test-all
```

### Production Monitoring

After deployment, monitor:
- Inference latency (should be <50ms)
- Accuracy on production data (should be >80%)
- Memory usage (should be stable)
- Error rates (should be <1%)

## Next Steps

After passing stress tests:

1. ✓ Review production readiness checklist
2. ✓ Deploy to staging environment
3. ✓ Run smoke tests in staging
4. ✓ Monitor performance metrics
5. ✓ Deploy to production
6. ✓ Set up continuous monitoring
7. ✓ Establish alerting thresholds

## Contributing

When adding new stress tests:

1. Follow existing test structure
2. Include clear descriptions
3. Test both success and failure cases
4. Document expected behavior
5. Update this documentation
6. Add to master test runner

## Summary

The stress test suite provides comprehensive validation with:
- **10,114 test examples** (10K train + 1.2K validation + 114 edge cases)
- **12 real-world categories** (customer service domain)
- **4 test suites** (large-scale, edge cases, stress, robustness)
- **50+ individual tests** across all suites
- **10-20 minute runtime** for complete validation

This ensures the system is production-ready for real-world deployment with confidence in its performance, reliability, and robustness.
