CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
INCLUDES = -I.

# Main binaries
TRAIN_BIN = gladtotext
INFER_BIN = gladtotext-infer
COMPACT_TRAIN_BIN = gladtotext-compact
COMPACT_INFER_BIN = gladtotext-compact-infer
TINY_TRAIN_BIN = gladtotext-tiny
SIZE_CALC = model-size-calculator

# Test binaries
UNIT_TEST_BINS = tests/unit/t1 tests/unit/t2 tests/unit/t3 tests/unit/t4 tests/unit/t5
INTEGRATION_TESTS = tests/integration/test_full_pipeline.sh \
                    tests/integration/test_sentence_encoding.sh \
                    tests/integration/test_transfer_learning.sh

.PHONY: all clean test test-unit test-integration test-comprehensive install compact tiny tools examples python python-install python-test

all: $(TRAIN_BIN) $(INFER_BIN)

compact: $(COMPACT_TRAIN_BIN) $(COMPACT_INFER_BIN)

tiny: $(TINY_TRAIN_BIN)

tools: $(SIZE_CALC)

$(TRAIN_BIN): main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(INFER_BIN): inference.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(COMPACT_TRAIN_BIN): compact_model.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(COMPACT_INFER_BIN): compact_infer.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(TINY_TRAIN_BIN): tiny_model.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

$(SIZE_CALC): model_size_calculator.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

# Tests
tests/unit/t1: tests/unit/test_backbone.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/unit/t2: tests/unit/test_dictionary.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/unit/t3: tests/unit/test_unsupervised.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/unit/t4: tests/unit/test_supervised.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/unit/t5: tests/unit/test_memory.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

test-unit: $(UNIT_TEST_BINS)
	@echo "Running unit tests..."
	@./tests/unit/t1 && ./tests/unit/t2 && ./tests/unit/t3 && ./tests/unit/t4 && ./tests/unit/t5
	@echo "✓ All unit tests passed!"

test-integration: all
	@echo "Running integration tests..."
	@cd tests/integration && ./test_full_pipeline.sh
	@cd tests/integration && ./test_sentence_encoding.sh
	@cd tests/integration && ./test_transfer_learning.sh
	@echo "✓ All integration tests passed!"

test-comprehensive: all
	@echo "Running comprehensive test suite..."
	@cd tests/comprehensive && bash run_all_tests.sh

test-stress: all
	@echo "Running stress test suite..."
	@cd tests/stress && bash run_stress_tests.sh

test: test-unit test-integration
	@echo ""
	@echo "=========================================="
	@echo "✓ ALL TESTS PASSED!"
	@echo "=========================================="

test-all: test-unit test-integration test-comprehensive test-stress
	@echo ""
	@echo "=========================================="
	@echo "✓ ALL TESTS PASSED (INCLUDING COMPREHENSIVE AND STRESS)!"
	@echo "=========================================="

clean:
	rm -f $(TRAIN_BIN) $(INFER_BIN) $(COMPACT_TRAIN_BIN) $(COMPACT_INFER_BIN) $(TINY_TRAIN_BIN) $(SIZE_CALC) $(UNIT_TEST_BINS)
	rm -f *.bin *.o *.config
	rm -f tests/integration/*.txt tests/integration/*.bin

install: all compact
	@echo "Installing to /usr/local/bin (requires sudo)"
	sudo cp $(TRAIN_BIN) /usr/local/bin/
	sudo cp $(INFER_BIN) /usr/local/bin/
	sudo cp $(COMPACT_TRAIN_BIN) /usr/local/bin/
	sudo cp $(COMPACT_INFER_BIN) /usr/local/bin/

# Examples
examples:
	@echo "Available examples:"
	@echo "  make example-quickstart          - Quick demo"
	@echo "  make example-intent              - Intent classification comparison"
	@echo "  make example-transfer            - Transfer learning"
	@echo "  make example-compact             - Compact models"
	@echo "  make example-sentence            - Sentence encoding"

example-quickstart:
	@cd examples && ./quickstart.sh

example-intent:
	@cd examples && ./example_intent_classification.sh

example-transfer:
	@cd examples && ./example_transfer_learning.sh

example-compact:
	@cd examples && ./demo_compact.sh

example-sentence:
	@cd examples && ./demo_sentence_encoding.sh

# Python bindings
python:
	@echo "Building Python bindings..."
	@echo "Make sure pybind11 is installed: pip install pybind11"
	pip install .

python-install:
	@echo "Installing Python package..."
	pip install .

python-test: python-install
	@echo "Testing Python bindings..."
	@python3 python/example_usage.py

python-dev:
	@echo "Installing in development mode..."
	pip install -e .
