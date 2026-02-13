# Stress Test Quick Start Guide

## Prerequisites

1. Build the binaries:
   ```bash
   cd /path/to/GLADtoTEXT
   make clean
   make all
   ```

2. Verify binaries exist:
   ```bash
   ls -lh gladtotext gladtotext-infer
   ```

3. Check disk space (need ~20GB free):
   ```bash
   df -h .
   ```

## Running Tests

### Option 1: Diagnostic First (Recommended)

Run the diagnostic script to identify any issues:

```bash
cd tests/stress
./diagnose_test4.sh
```

This will test each component individually and show exactly what's failing.

### Option 2: Individual Test Suites

Run each test suite separately:

```bash
cd tests/stress

# Clean up first
rm -f *.bin

# Test 1: Large-scale classification (10K examples, 12 classes)
# Expected time: ~15 minutes
./test_large_scale.sh

# Test 2: Edge cases (typos, negations, special characters)
# Expected time: ~5 minutes
./test_edge_cases.sh

# Test 3: Stress testing (memory, concurrency, large batches)
# Expected time: ~10 minutes
./test_stress.sh

# Test 4: Robustness (variations, tolerance)
# Expected time: ~5 minutes
./test_robustness.sh
```

### Option 3: Full Test Suite

Run all tests together:

```bash
cd tests/stress
rm -f *.bin  # Clean up first
./run_stress_tests.sh
```

Expected total time: 10-20 minutes

## Understanding Results

### Success Output
```
✓ Test PASSED
```

### Failure Output (Enhanced)
```
✗ Test failed (exit code: 1)
   Error output:
   [Last 10-20 lines of error messages]
```

### Warning Output
```
⚠ Warning message
```
Warnings are expected for some edge cases and don't indicate failure.

## Common Issues

### Issue 1: Binaries Not Found
```
Error: gladtotext binaries not found!
```

**Solution**: Run `make all` in the root directory.

### Issue 2: Data Files Not Found
```
Error: Test data files not found!
```

**Solution**: Ensure `tests/data/large_scale_train.txt` exists.

### Issue 3: Disk Space
```
✗ Training failed
   Error: Cannot write model file
```

**Solution**: Free up disk space. Each model can be 1GB+.

### Issue 4: Memory Issues
```
✗ dim=300 failed
   Error: std::bad_alloc
```

**Solution**: Reduce dimensions or increase available RAM.

### Issue 5: Attention Mode Failure
```
✗ Optimized training failed
   Error: Unknown attention mode
```

**Solution**: Check if `-attention simple` is supported in your build.

## Cleanup

After running tests, clean up large model files:

```bash
cd tests/stress
rm -f *.bin
ls -lh  # Verify cleanup
```

## Interpreting Test Results

### Large-Scale Test
- Tests training with 10K examples across 12 classes
- Validates batch processing, early stopping, and inference
- Creates models up to 1.1GB in size

### Edge Case Test
- Tests robustness with typos, negations, and special characters
- Some warnings (⚠) are expected and normal
- Validates model handles real-world messy input

### Stress Test
- Tests memory limits with large dimensions (300, 500)
- Tests concurrent inference (10 parallel processes)
- Tests rapid sequential inference (1000 predictions)

### Robustness Test
- Compares bag-of-words vs sentence encoding
- Tests case sensitivity, word order, and variations
- Validates consistent predictions across input variations

## Getting Help

If tests fail:

1. Run `./diagnose_test4.sh` to identify the specific issue
2. Check the error output (now displayed with enhanced reporting)
3. Verify disk space: `df -h .`
4. Verify memory: `free -h` (Linux) or `vm_stat` (macOS)
5. Check the FIXES_APPLIED.md document for known issues

## Performance Expectations

On a typical development machine:
- Large-scale test: 10-15 minutes
- Edge case test: 3-5 minutes
- Stress test: 8-12 minutes
- Robustness test: 3-5 minutes
- Total: 25-35 minutes

Faster machines with more RAM and CPU cores will complete sooner.
