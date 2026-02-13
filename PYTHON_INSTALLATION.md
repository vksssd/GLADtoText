# Python Installation Guide

## Quick Install

### Method 1: Install from Local Directory (Development)

```bash
# Clone the repository
git clone https://github.com/yourusername/GLADtoTEXT.git
cd GLADtoTEXT

# Install dependencies
pip install pybind11 numpy

# Install the package
pip install .

# Or install in editable mode for development
pip install -e .
```

### Method 2: Install from GitHub (Direct)

```bash
# Install latest from main branch
pip install git+https://github.com/yourusername/GLADtoTEXT.git

# Install specific version/tag
pip install git+https://github.com/yourusername/GLADtoTEXT.git@v0.1.0

# Install with development dependencies
pip install "gladtotext[dev] @ git+https://github.com/yourusername/GLADtoTEXT.git"
```

### Method 3: Install from PyPI (When Published)

```bash
# Simple install
pip install gladtotext

# With development dependencies
pip install gladtotext[dev]
```

## Prerequisites

### System Requirements

- Python 3.7 or higher
- C++17 compatible compiler (g++, clang, MSVC)
- pip 19.0 or higher

### Compiler Setup

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential python3-dev

# Fedora/RHEL
sudo dnf install gcc-c++ python3-devel
```

**macOS:**
```bash
# Install Xcode Command Line Tools
xcode-select --install
```

**Windows:**
- Install Visual Studio 2019 or later with C++ support
- Or install Build Tools for Visual Studio

## Verification

After installation, verify it works:

```bash
# Check if module can be imported
python3 -c "import gladtotext; print('GLADtoTEXT version:', gladtotext.__version__ if hasattr(gladtotext, '__version__') else '0.1.0')"

# Run example (requires a trained model)
python3 python/example_usage.py
```

## Building from Source

### Step-by-Step

1. **Install build dependencies:**
   ```bash
   pip install build wheel pybind11
   ```

2. **Build source distribution:**
   ```bash
   python3 -m build --sdist
   ```

3. **Build wheel:**
   ```bash
   python3 -m build --wheel
   ```

4. **Install the wheel:**
   ```bash
   pip install dist/gladtotext-*.whl
   ```

### Verify Build Contents

Check that header files are included:

```bash
# Extract and inspect source distribution
tar -tzf dist/gladtotext-*.tar.gz | grep "\.h$"

# Should show:
# gladtotext-0.1.0/core/config.h
# gladtotext-0.1.0/core/dictionary.h
# ... etc
```

## Development Setup

For contributing or development:

```bash
# Clone repository
git clone https://github.com/yourusername/GLADtoTEXT.git
cd GLADtoTEXT

# Create virtual environment
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install in editable mode with dev dependencies
pip install -e ".[dev]"

# Install pre-commit hooks (if available)
# pre-commit install
```

## Testing the Installation

### Basic Test

```python
import gladtotext
import numpy as np

# Create model instance
model = gladtotext.Model()
print("✓ Module imported successfully")

# Try loading a model (will fail if no model exists, which is OK)
try:
    model.load_model("test_model.bin")
    print("✓ Model loading works")
except:
    print("⚠ No test model found (this is OK)")
```

### Full Test with Training

```bash
# 1. Build C++ binaries first
make all

# 2. Train a test model
cat > test_train.txt << 'EOF'
__positive great product
__positive love it
__negative terrible
__negative hate it
EOF

./gladtotext supervised -input test_train.txt -output test_model -dim 30 -epoch 10

# 3. Test Python bindings
python3 << 'EOF'
import gladtotext

model = gladtotext.Model()
assert model.load_model("test_model.bin"), "Failed to load model"

info = model.get_info()
print(f"✓ Model loaded: {info['num_labels']} labels, dim={info['dim']}")

predictions = model.predict("great", k=2)
print(f"✓ Prediction works: {predictions}")

vec = model.get_word_vector("great")
print(f"✓ Word vector shape: {vec.shape}")

print("\n✅ All tests passed!")
EOF

# 4. Cleanup
rm test_train.txt test_model.bin
```

## Troubleshooting

### Issue: "No module named 'pybind11'"

```bash
pip install pybind11
```

### Issue: "No module named 'gladtotext'"

The package isn't installed. Run:
```bash
pip install .
```

### Issue: Compilation Errors

**Missing compiler:**
```bash
# Linux
sudo apt-get install build-essential

# macOS
xcode-select --install
```

**Wrong C++ standard:**
Ensure your compiler supports C++17. Update if necessary.

### Issue: "fatal error: 'core/config.h' file not found"

This means MANIFEST.in isn't working. Verify:
```bash
# Check if MANIFEST.in exists
ls -la MANIFEST.in

# Rebuild with verbose output
pip install -v .
```

### Issue: Import works but functions fail

The C++ binaries might not be built. Build them first:
```bash
make all
```

### Issue: "Symbol not found" or "undefined symbol"

This is usually a linking issue. Try:
```bash
# Clean and rebuild
pip uninstall gladtotext
pip cache purge
pip install . --no-cache-dir
```

## Platform-Specific Notes

### Linux

- Works out of the box on most distributions
- May need `python3-dev` package
- Use system Python or virtual environment

### macOS

- Requires Xcode Command Line Tools
- Works with both Intel and Apple Silicon
- May need to set `ARCHFLAGS` for universal builds:
  ```bash
  ARCHFLAGS="-arch arm64 -arch x86_64" pip install .
  ```

### Windows

- Requires Visual Studio or Build Tools
- Use Python from python.org (not Microsoft Store)
- May need to run from "Developer Command Prompt"

## Uninstallation

```bash
pip uninstall gladtotext
```

## Upgrading

```bash
# From local directory
pip install --upgrade .

# From GitHub
pip install --upgrade git+https://github.com/yourusername/GLADtoTEXT.git

# From PyPI (when available)
pip install --upgrade gladtotext
```

## Virtual Environments

Recommended for development:

```bash
# Create virtual environment
python3 -m venv gladtotext-env

# Activate
source gladtotext-env/bin/activate  # Linux/macOS
# or
gladtotext-env\Scripts\activate  # Windows

# Install
pip install .

# Deactivate when done
deactivate
```

## Docker Installation

For containerized environments:

```dockerfile
FROM python:3.11-slim

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install GLADtoTEXT
RUN pip install git+https://github.com/yourusername/GLADtoTEXT.git

# Verify installation
RUN python3 -c "import gladtotext; print('GLADtoTEXT installed')"
```

## Next Steps

After installation:

1. Read [PYTHON_GUIDE.md](PYTHON_GUIDE.md) for API documentation
2. Try [python/example_usage.py](python/example_usage.py)
3. Train your first model with the C++ CLI
4. Use the Python API for inference

## Getting Help

If you encounter issues:

1. Check this troubleshooting section
2. Search existing GitHub issues
3. Create a new issue with:
   - Your OS and Python version
   - Full error message
   - Steps to reproduce
