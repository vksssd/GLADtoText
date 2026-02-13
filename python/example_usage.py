#!/usr/bin/env python3
"""
Example usage of GLADtoTEXT Python bindings
"""

import gladtotext
import numpy as np

def main():
    # Load a trained model
    model = gladtotext.Model()
    
    print("Loading model...")
    if not model.load_model("model.bin"):
        print("Error: Could not load model.bin")
        print("Train a model first using: ./gladtotext supervised -input train.txt -output model")
        return
    
    # Get model info
    info = model.get_info()
    print(f"\nModel Info:")
    print(f"  Vocabulary size: {info['vocab_size']}")
    print(f"  Embedding dimension: {info['dim']}")
    print(f"  Supervised: {info['supervised']}")
    print(f"  Sentence encoder: {info['sentence_encoder']}")
    
    if info['supervised']:
        print(f"  Number of labels: {info['num_labels']}")
        print(f"  Labels: {info['labels']}")
    
    # Get word vectors
    print("\n--- Word Vectors ---")
    words = ["hello", "world", "python"]
    for word in words:
        try:
            vec = model.get_word_vector(word)
            print(f"{word}: {vec[:5]}... (shape: {vec.shape})")
        except Exception as e:
            print(f"Error getting vector for '{word}': {e}")
    
    # Predict (if supervised model)
    if info['supervised']:
        print("\n--- Predictions ---")
        test_texts = [
            "hello there",
            "goodbye friend",
            "how are you"
        ]
        
        for text in test_texts:
            predictions = model.predict(text, k=2)
            print(f"\nText: '{text}'")
            for label, score in predictions:
                print(f"  {label}: {score:.4f}")


if __name__ == "__main__":
    main()
