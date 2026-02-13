# Comprehensive Test Suite

This directory contains comprehensive tests for GLADtoTEXT with larger datasets and multiple classification tasks.

## Test Datasets

### 1. Sentiment Classification (3 classes)
- **Training:** 60 examples (20 per class)
- **Validation:** 15 examples (5 per class)
- **Classes:** positive, negative, neutral
- **File:** `../data/sentiment_large.txt`

### 2. Intent Classification (8 classes)
- **Training:** 120 examples (15 per class)
- **Validation:** 16 examples (2 per class)
- **Classes:** 
  - book_flight
  - book_hotel
  - cancel_booking
  - check_status
  - get_weather
  - greeting
  - farewell
  - help_request
- **File:** `../data/intent_classification.txt`

### 3. Topic Classification (8 classes)
- **Training:** 120 examples (15 per class)
- **Validation:** 16 examples (2 per class)
- **Classes:**
  - sports
  - technology
  - politics
  - business
  - science
  - entertainment
  - health
  - education
- **File:** `../data/topic_classification.txt`

## Test Scripts

### Individual Tests

1. **test_sentiment_classification.sh**
   - Tests 3-class sentiment classification
   - Validates training, validation, batch processing
   - Tests all attention modes
   - Verifies inference for all classes

2. **test_intent_classification.sh**
   - Tests 8-class intent classification
   - Tests different batch sizes (1, 16, 32, 64)
   - Validates sentence encoding mode
   - Verifies inference for all intents

3. **test_topic_classification.sh**
   - Tests 8-class topic classification
   - Tests different dimensions (50, 100, 150, 200)
   - Tests optimized configurations
   - Verifies inference for all topics

### Master Test Runner

**run_all_tests.sh**
- Runs all comprehensive tests
- Performs performance benchmarks
- Reports summary statistics

## Running Tests

### Run All Tests
```bash
cd tests/comprehensive
bash run_all_tests.sh
```

### Run Individual Tests
```bash
cd tests/comprehensive
bash test_sentiment_classification.sh
bash test_intent_classification.sh
bash test_topic_classification.sh
```

## Test Coverage

### Features Tested
- ✅ Multi-class classification (3, 8 classes)
- ✅ Training with validation
- ✅ Early stopping
- ✅ Batch processing (1, 16, 32, 64)
- ✅ Attention modes (full, simple, none)
- ✅ Sentence encoding
- ✅ Different dimensions (50, 100, 150, 200)
- ✅ Grammar and phonetic features
- ✅ Model serialization and loading
- ✅ Inference accuracy

### Performance Benchmarks
- Baseline training speed
- Batch processing speedup
- Attention mode speedup
- Combined optimizations

## Expected Results

### Training Time (approximate)
- Sentiment (60 examples): ~2-5 seconds
- Intent (120 examples): ~5-10 seconds
- Topic (120 examples): ~5-10 seconds

### Accuracy (approximate)
- Sentiment: 80-95% on validation
- Intent: 85-95% on validation
- Topic: 75-90% on validation

### Performance Gains
- Batch processing: 30-50% faster
- Simple attention: 5-15% faster
- Combined: 40-60% faster

## Troubleshooting

### Tests Fail
1. Check that GLADtoTEXT is compiled: `make all`
2. Verify data files exist in `tests/data/`
3. Check available disk space for model files
4. Review error messages in test output

### Slow Performance
1. Reduce epochs for faster testing
2. Use smaller dimensions
3. Enable batch processing
4. Use simple or no attention mode

### Low Accuracy
1. Increase number of epochs
2. Increase model dimension
3. Adjust learning rate
4. Check data quality

## Adding New Tests

To add a new test:

1. Create training data in `tests/data/`
2. Create validation data in `tests/data/`
3. Create test script in `tests/comprehensive/`
4. Add test to `run_all_tests.sh`

Example test structure:
```bash
#!/bin/bash
echo "=== My Test ==="
# 1. Train model
# 2. Test with validation
# 3. Test inference
# 4. Cleanup
echo "=== Test passed! ==="
```

## Dataset Statistics

### Sentiment Dataset
- Total examples: 60
- Average words per example: 8-12
- Vocabulary size: ~150 unique words
- Label distribution: Balanced (20 each)

### Intent Dataset
- Total examples: 120
- Average words per example: 6-10
- Vocabulary size: ~200 unique words
- Label distribution: Balanced (15 each)

### Topic Dataset
- Total examples: 120
- Average words per example: 10-15
- Vocabulary size: ~300 unique words
- Label distribution: Balanced (15 each)

## Notes

- All tests clean up model files after completion
- Tests redirect training output to /dev/null for cleaner output
- Inference tests check for correct label in output
- Performance benchmarks use `time` command
- Tests are designed to run quickly (< 5 minutes total)

## Requirements

- GLADtoTEXT compiled and working
- Bash shell
- ~500MB free disk space for temporary model files
- ~5 minutes for full test suite

## Success Criteria

All tests should:
- ✅ Complete without errors
- ✅ Train models successfully
- ✅ Produce correct predictions
- ✅ Show decreasing loss
- ✅ Clean up temporary files

## Contact

For issues or questions about the test suite, please refer to the main README.md or open an issue on GitHub.
