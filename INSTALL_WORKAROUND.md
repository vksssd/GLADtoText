# Installation Workaround Guide

## Quick Fix (Works Now)

If you're getting errors installing from GitHub, use this method:

```bash
# Step 1: Install dependencies first
pip install pybind11 numpy

# Step 2: Install GLADtoTEXT
pip install git+https://github.com/vksssd/GLADtoTEXT.git
```

That's it! This works around the build dependency issue.

## Alternative Methods

### Method 1: Clone and Install (Most Reliable)

```bash
# Clone the repository
git clone https://github.com/vksssd/GLADtoTEXT.git
cd GLADtoTEXT

# Install dependencies
pip install pybind11 numpy

# Install the package
pip install .
```

### Method 2: Use requirements.txt

```bash
# Create requirements.txt
cat > requirements.txt << 'EOF'
pybind11>=2.10.0
numpy>=1.19.0
git+https://github.com/vksssd/GLADtoTEXT.git
EOF

# Install everything
pip install -r requirements.txt
```

### Method 3: No Build Isolation

```bash
# Install dependencies
pip install pybind11 numpy

# Install without build isolation
pip install --no-build-isolation git+https://github.com/vksssd/GLADtoTEXT.git
```

## Complete Example

Here's a complete workflow from scratch:

```bash
# Create a new project directory
mkdir my_project
cd my_project

# Create virtual environment (recommended)
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install pybind11 numpy

# Install GLADtoTEXT
pip install git+https://github.com/vksssd/GLADtoTEXT.git

# Verify installation
python3 << 'EOF'
import gladtotext
print("✓ GLADtoTEXT installed successfully!")

# Test basic functionality
model = gladtotext.Model()
print("✓ Model class works!")
EOF
```

## For Docker Users

```dockerfile
FROM python:3.11-slim

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install Python dependencies first
RUN pip install --no-cache-dir pybind11 numpy

# Install GLADtoTEXT
RUN pip install --no-cache-dir git+https://github.com/vksssd/GLADtoTEXT.git

# Verify installation
RUN python3 -c "import gladtotext; print('GLADtoTEXT installed')"
```

## For Conda Users

```bash
# Create conda environment
conda create -n gladtotext python=3.11
conda activate gladtotext

# Install dependencies via conda
conda install -c conda-forge pybind11 numpy

# Install GLADtoTEXT via pip
pip install git+https://github.com/vksssd/GLADtoTEXT.git
```

## Troubleshooting

### Error: "No module named 'pybind11'"

**Solution:** Install pybind11 first:
```bash
pip install pybind11
```

### Error: "No module named 'numpy'"

**Solution:** Install numpy first:
```bash
pip install numpy
```

### Error: "fatal error: 'pybind11/pybind11.h' file not found"

**Solution:** Install pybind11 development headers:
```bash
pip install pybind11[global]
```

### Error: "Microsoft Visual C++ 14.0 or greater is required" (Windows)

**Solution:** Install Visual Studio Build Tools:
1. Download from https://visualstudio.microsoft.com/downloads/
2. Install "Desktop development with C++"
3. Restart terminal and try again

### Error: "command 'gcc' failed" (Linux)

**Solution:** Install build tools:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential python3-dev

# Fedora/RHEL
sudo dnf install gcc-c++ python3-devel
```

### Error: "command 'clang' failed" (macOS)

**Solution:** Install Xcode Command Line Tools:
```bash
xcode-select --install
```

## Why This Workaround is Needed

The issue is with pip's isolated build environment. When you run:
```bash
pip install git+https://...
```

Pip creates an isolated environment to build the package, but it doesn't have `pybind11` installed yet. By installing `pybind11` first in your main environment, the build process can find it.

## When Will This Be Fixed?

The maintainer has updated the build configuration. After the next commit is pushed to GitHub, direct installation should work without the workaround.

## Recommended Approach

For now, always install dependencies first:

```bash
# Add this to your project's requirements.txt
pybind11>=2.10.0
numpy>=1.19.0
git+https://github.com/vksssd/GLADtoTEXT.git
```

Then install with:
```bash
pip install -r requirements.txt
```

This ensures dependencies are installed in the correct order.

## Testing Your Installation

After installation, run this test:

```bash
python3 << 'EOF'
import gladtotext
import numpy as np

print("Testing GLADtoTEXT installation...")

# Test 1: Module import
print("✓ Module imported")

# Test 2: Create model instance
model = gladtotext.Model()
print("✓ Model instance created")

# Test 3: Check methods exist
assert hasattr(model, 'load_model'), "load_model method missing"
assert hasattr(model, 'predict'), "predict method missing"
assert hasattr(model, 'get_word_vector'), "get_word_vector method missing"
assert hasattr(model, 'get_info'), "get_info method missing"
print("✓ All methods present")

print("\n✅ Installation successful!")
print("\nNext steps:")
print("1. Train a model using the C++ CLI")
print("2. Load it in Python with model.load_model('model.bin')")
print("3. Use model.predict() for inference")
EOF
```

## Getting Help

If you still have issues:

1. Check you have Python 3.7+: `python3 --version`
2. Check you have a C++ compiler: `gcc --version` or `clang --version`
3. Try the clone-and-install method (most reliable)
4. Open an issue on GitHub with your error message

## Summary

**TL;DR:** Install dependencies first, then install GLADtoTEXT:

```bash
pip install pybind11 numpy
pip install git+https://github.com/vksssd/GLADtoTEXT.git
```
