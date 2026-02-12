CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
INCLUDES = -I.

# Main binaries
TRAIN_BIN = gladtotext
INFER_BIN = gladtotext-infer
COMPACT_TRAIN_BIN = gladtotext-compact
COMPACT_INFER_BIN = gladtotext-compact-infer
TINY_TRAIN_BIN = gladtotext-tiny

# Test binaries
TEST_BINS = tests/t1 tests/t2 tests/t3 tests/t4 tests/t5

.PHONY: all clean test install compact tiny

all: $(TRAIN_BIN) $(INFER_BIN)

compact: $(COMPACT_TRAIN_BIN) $(COMPACT_INFER_BIN)

tiny: $(TINY_TRAIN_BIN)

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

# Tests
tests/t1: tests/test_backbone.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/t2: tests/test_dictionary.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/t3: tests/test_unsupervised.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/t4: tests/test_supervised.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

tests/t5: tests/test_memory.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

test: $(TEST_BINS)
	@echo "Running tests..."
	@./tests/t1 && ./tests/t2 && ./tests/t3 && ./tests/t4 && ./tests/t5
	@echo "All tests passed!"

clean:
	rm -f $(TRAIN_BIN) $(INFER_BIN) $(COMPACT_TRAIN_BIN) $(COMPACT_INFER_BIN) $(TINY_TRAIN_BIN) $(TEST_BINS)
	rm -f *.bin *.o

install: all compact
	@echo "Installing to /usr/local/bin (requires sudo)"
	sudo cp $(TRAIN_BIN) /usr/local/bin/
	sudo cp $(INFER_BIN) /usr/local/bin/
	sudo cp $(COMPACT_TRAIN_BIN) /usr/local/bin/
	sudo cp $(COMPACT_INFER_BIN) /usr/local/bin/

# Example targets
example-cbow:
	@echo "Training CBOW model..."
	./$(TRAIN_BIN) cbow -input sample_train.txt -output example_model -dim 50 -epoch 5 -minCount 1

example-supervised:
	@echo "Training supervised model..."
	./$(TRAIN_BIN) supervised -input sample_supervised.txt -output example_classifier -dim 30 -epoch 10 -minCount 1

example-vectors:
	@echo "Getting word vectors..."
	echo "dog cat fox" | ./$(INFER_BIN) print-word-vector example_model.bin

example-predict:
	@echo "Predicting labels..."
	echo "this is a great movie" | ./$(INFER_BIN) predict example_classifier.bin 2


# Compact model examples
example-compact:
	@echo "Training compact intent classifier..."
	@cat > compact_demo.txt << 'EOF' && \
	__label__book_flight book a flight to paris; \
	__label__book_hotel find a hotel in rome; \
	__label__cancel cancel my booking; \
	__label__status check my reservation; \
	EOF
	./$(COMPACT_TRAIN_BIN) compact_demo.txt compact_demo.bin 20 50 0.2
	@echo ""
	@echo "Testing predictions..."
	@echo "book flight to london" | ./$(COMPACT_INFER_BIN) compact_demo.bin 1
	@echo ""
	@ls -lh compact_demo.bin
