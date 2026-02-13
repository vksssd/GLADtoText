#!/usr/bin/env python3
"""
Correct workflow for GLADtoTEXT Python bindings

IMPORTANT: Training is done via C++ CLI, Python is for inference only.
"""

import gladtotext
import sys
import os

def main():
    model_path = "chatbot.bin"
    
    # Check if model exists
    if not os.path.exists(model_path):
        print(f"Error: Model file '{model_path}' not found!")
        print()
        print("You need to train a model first using the C++ CLI:")
        print()
        print("  # Create training data")
        print("  cat > train.txt << 'EOF'")
        print("  __greeting hello")
        print("  __greeting hi there")
        print("  __farewell goodbye")
        print("  __farewell see you")
        print("  EOF")
        print()
        print("  # Train model")
        print("  ./gladtotext supervised -input train.txt -output chatbot -dim 50 -epoch 20")
        print()
        print("Then run this script again.")
        sys.exit(1)
    
    # Load model
    print(f"Loading model from {model_path}...")
    model = gladtotext.Model()
    
    if not model.load_model(model_path):
        print(f"Failed to load model from {model_path}")
        sys.exit(1)
    
    # Get model info
    info = model.get_info()
    print(f"✓ Model loaded successfully")
    print(f"  Labels: {info['num_labels']}")
    print(f"  Dimension: {info['dim']}")
    print(f"  Categories: {', '.join(info['labels'])}")
    print()
    
    # Test predictions
    test_texts = [
        "hello friend",
        "goodbye everyone",
        "hi there",
        "see you later"
    ]
    
    print("Making predictions:")
    print("-" * 50)
    
    for text in test_texts:
        predictions = model.predict(text, k=1)
        label, score = predictions[0]
        print(f"'{text}' → {label} (score: {score:.4f})")
    
    print()
    print("✓ All predictions completed successfully")

if __name__ == "__main__":
    main()
