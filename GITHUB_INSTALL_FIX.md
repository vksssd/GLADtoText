# GitHub Installation Fix

## The Problem

When installing from GitHub with:
```bash
pip install git+https://github.com/vksssd/GLADtoTEXT.git
```

You get an error:
```
error: subprocess-exited-with-error
× Getting requirements to build wheel did not run successfully.
```

## Root Cause

The build system needs `pybind11` to be installed before it can determine build requirements, but pip's isolated build environment doesn't have it yet.

## Solution Applied

Updated `pyproject.toml` and `setup.py` to properly declare build dependencies.

## How to Install Now

### Option 1: Install Dependencies First (Recommended)

```bash
# Install build dependencies
pip install pybind11 numpy

# Then install GLADtoTEXT
pip install git+https://github.com/vksssd/GLADtoTEXT.git
```

### Option 2: Use --no-build-isolation (Alternative)

```bash
# Install dependencies
pip install pybind11 numpy

# Install without build isolation
pip install --no-build-isolation git+https://github.com/vksssd/GLADtoTEXT.git
```

### Option 3: Clone and Install Locally

```bash
# Clone repository
git clone https://github.com/vksssd/GLADtoTEXT.git
cd GLADtoTEXT

# Install dependencies
pip install pybind11 numpy

# Install package
pip install .
```

## Verification

After installation, verify it works:

```bash
python3 -c "import gladtotext; print('Success!')"
```

## Why This Happens

Modern pip uses isolated build environments (PEP 517/518). The build system needs to:
1. Read `pyproject.toml` to know what to install
2. But `setup.py` imports `pybind11` before pip can install it
3. This creates a chicken-and-egg problem

## The Fix

We've updated the configuration to:
1. Declare `pybind11` in `pyproject.toml` build requirements
2. Add `setup_requires` in `setup.py` as fallback
3. Handle ImportError gracefully if pybind11 isn't available yet

## Testing

After pushing the fix to GitHub, test with:

```bash
# Create fresh virtual environment
python3 -m venv test-env
source test-env/bin/activate

# Try direct install (should work now)
pip install git+https://github.com/vksssd/GLADtoTEXT.git

# Verify
python3 -c "import gladtotext; print('Success!')"

# Cleanup
deactivate
rm -rf test-env
```

## For Users

Until the fix is pushed to GitHub, use Option 1 (install dependencies first).

After the fix is pushed, direct installation should work:
```bash
pip install git+https://github.com/vksssd/GLADtoTEXT.git
```

## Alternative: Pre-built Wheels

For easier installation, consider building and distributing wheels:

```bash
# Build wheel
python3 -m build --wheel

# Upload to GitHub Releases
# Users can then download and install:
pip install gladtotext-0.1.0-*.whl
```

Or publish to PyPI for the easiest installation:
```bash
pip install gladtotext
```

## Next Steps

1. Commit and push the updated `setup.py` and `pyproject.toml`
2. Test installation from GitHub
3. If issues persist, consider publishing to PyPI
4. Or provide pre-built wheels in GitHub Releases
