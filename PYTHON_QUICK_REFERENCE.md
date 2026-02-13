# Python Package - Quick Reference

## Installation

```bash
# Local (development)
pip install .

# GitHub (sharing)
pip install git+https://github.com/yourusername/GLADtoTEXT.git

# PyPI (when published)
pip install gladtotext
```

## Basic Usage

```python
import gladtotext

# Load model
model = gladtotext.Model()
model.load_model("model.bin")

# Predict
predictions = model.predict("your text", k=3)
for label, score in predictions:
    print(f"{label}: {score:.4f}")

# Word vector
vec = model.get_word_vector("hello")
print(vec.shape)  # (dim,)

# Model info
info = model.get_info()
print(info)
```

## Training (C++ CLI)

```bash
# Train model
./gladtotext supervised -input train.txt -output model -dim 50 -epoch 20

# Use in Python
python3 -c "
import gladtotext
model = gladtotext.Model()
model.load_model('model.bin')
print(model.predict('test text'))
"
```

## Testing

```bash
# Run all tests
./test_python_package.sh

# Quick test
python3 -c "import gladtotext; print('OK')"
```

## Files Modified

- ✅ `MANIFEST.in` - Created (includes headers)
- ✅ `setup.py` - Enhanced metadata
- ✅ `pyproject.toml` - Updated config

## Documentation

- `PYTHON_PACKAGE_SUMMARY.md` - Complete overview
- `PYTHON_INSTALLATION.md` - Installation guide
- `PYPI_PUBLISHING_GUIDE.md` - Publishing steps
- `python/README.md` - API reference

## Status

✅ Ready for GitHub installation
🔜 Ready for PyPI (when you want)

## Next Steps

1. Run `./test_python_package.sh`
2. Update GitHub URLs in setup.py
3. Push to GitHub
4. Test: `pip install git+https://github.com/...`
5. Publish to PyPI when ready
