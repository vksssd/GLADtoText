# PyPI Publishing Guide

## Overview

This guide walks you through publishing GLADtoTEXT to PyPI (Python Package Index).

## Prerequisites

### 1. Create PyPI Accounts

- **TestPyPI** (for testing): https://test.pypi.org/account/register/
- **PyPI** (production): https://pypi.org/account/register/

### 2. Install Publishing Tools

```bash
pip install build twine
```

### 3. Verify Package is Ready

Run the test script:
```bash
./test_python_package.sh
```

All tests should pass.

## Pre-Publishing Checklist

- [ ] All tests pass
- [ ] Version number is correct in `setup.py` and `pyproject.toml`
- [ ] README.md is comprehensive and accurate
- [ ] LICENSE file exists
- [ ] Author information is updated (not placeholder)
- [ ] GitHub URLs are correct
- [ ] MANIFEST.in includes all necessary files
- [ ] No sensitive information in code
- [ ] CHANGELOG.md exists (optional but recommended)

## Step 1: Update Version Number

Edit both files to have the same version:

**setup.py:**
```python
__version__ = "0.1.0"  # Update this
```

**pyproject.toml:**
```toml
version = "0.1.0"  # Update this
```

Version numbering guide:
- `0.1.0` - Initial release
- `0.1.1` - Bug fixes
- `0.2.0` - New features (backward compatible)
- `1.0.0` - Stable release

## Step 2: Clean Previous Builds

```bash
# Remove old build artifacts
rm -rf build/ dist/ *.egg-info/

# Remove compiled Python files
find . -type d -name __pycache__ -exec rm -rf {} + 2>/dev/null || true
find . -type f -name "*.pyc" -delete
find . -type f -name "*.pyo" -delete
```

## Step 3: Build Distribution Files

```bash
# Build source distribution and wheel
python3 -m build

# Verify files were created
ls -lh dist/
```

You should see:
- `gladtotext-0.1.0.tar.gz` (source distribution)
- `gladtotext-0.1.0-*.whl` (wheel)

## Step 4: Verify Distribution Contents

### Check Source Distribution

```bash
# List contents
tar -tzf dist/gladtotext-*.tar.gz | head -20

# Verify headers are included
tar -tzf dist/gladtotext-*.tar.gz | grep "\.h$"
```

Should show header files like:
- `gladtotext-0.1.0/core/config.h`
- `gladtotext-0.1.0/core/dictionary.h`
- etc.

### Check Wheel

```bash
# List contents
unzip -l dist/gladtotext-*.whl | head -20
```

## Step 5: Test Installation Locally

```bash
# Create a test virtual environment
python3 -m venv test-env
source test-env/bin/activate

# Install from wheel
pip install dist/gladtotext-*.whl

# Test import
python3 -c "import gladtotext; print('Success!')"

# Deactivate and remove test environment
deactivate
rm -rf test-env
```

## Step 6: Upload to TestPyPI (Recommended First)

TestPyPI is a separate instance for testing. Always test here first!

```bash
# Upload to TestPyPI
python3 -m twine upload --repository testpypi dist/*

# You'll be prompted for:
# Username: __token__
# Password: <your TestPyPI API token>
```

### Get TestPyPI API Token

1. Go to https://test.pypi.org/manage/account/token/
2. Create a new API token
3. Copy the token (starts with `pypi-`)
4. Use it as password (username is `__token__`)

### Test Installation from TestPyPI

```bash
# Create fresh virtual environment
python3 -m venv test-pypi-env
source test-pypi-env/bin/activate

# Install from TestPyPI
pip install --index-url https://test.pypi.org/simple/ --extra-index-url https://pypi.org/simple/ gladtotext

# Test it works
python3 -c "import gladtotext; print('Success from TestPyPI!')"

# Cleanup
deactivate
rm -rf test-pypi-env
```

Note: We use `--extra-index-url` because dependencies (numpy, pybind11) are on regular PyPI.

## Step 7: Upload to PyPI (Production)

Once TestPyPI installation works:

```bash
# Upload to PyPI
python3 -m twine upload dist/*

# You'll be prompted for:
# Username: __token__
# Password: <your PyPI API token>
```

### Get PyPI API Token

1. Go to https://pypi.org/manage/account/token/
2. Create a new API token
3. Copy the token
4. Use it as password (username is `__token__`)

## Step 8: Verify on PyPI

1. Visit https://pypi.org/project/gladtotext/
2. Check that:
   - Description renders correctly
   - Links work
   - Version is correct
   - Classifiers are appropriate

## Step 9: Test Installation from PyPI

```bash
# Create fresh virtual environment
python3 -m venv test-real-env
source test-real-env/bin/activate

# Install from PyPI
pip install gladtotext

# Test it works
python3 -c "import gladtotext; print('Success from PyPI!')"

# Cleanup
deactivate
rm -rf test-real-env
```

## Step 10: Update Documentation

After publishing:

1. Update README.md with PyPI badge:
   ```markdown
   [![PyPI version](https://badge.fury.io/py/gladtotext.svg)](https://badge.fury.io/py/gladtotext)
   ```

2. Update installation instructions to use PyPI:
   ```bash
   pip install gladtotext
   ```

3. Create a GitHub release with the same version tag

## Automating with GitHub Actions

Create `.github/workflows/publish.yml`:

```yaml
name: Publish to PyPI

on:
  release:
    types: [published]

jobs:
  build-and-publish:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.11'
    
    - name: Install dependencies
      run: |
        pip install build twine
    
    - name: Build package
      run: python3 -m build
    
    - name: Publish to PyPI
      env:
        TWINE_USERNAME: __token__
        TWINE_PASSWORD: ${{ secrets.PYPI_API_TOKEN }}
      run: python3 -m twine upload dist/*
```

Then add your PyPI API token to GitHub Secrets as `PYPI_API_TOKEN`.

## Updating an Existing Package

To publish a new version:

1. Update version in `setup.py` and `pyproject.toml`
2. Update CHANGELOG.md
3. Clean and rebuild:
   ```bash
   rm -rf dist/ build/ *.egg-info/
   python3 -m build
   ```
4. Upload:
   ```bash
   python3 -m twine upload dist/*
   ```

## Troubleshooting

### Issue: "File already exists"

You can't re-upload the same version. Increment the version number.

### Issue: "Invalid distribution"

Check that:
- MANIFEST.in includes all necessary files
- setup.py has correct metadata
- No syntax errors in setup.py

### Issue: "Package installation fails"

Test locally first:
```bash
pip install dist/gladtotext-*.whl
```

Check error messages and fix before uploading.

### Issue: "README doesn't render"

- Ensure README.md is valid Markdown
- Check `long_description_content_type="text/markdown"` in setup.py
- Test rendering at https://readme.so/

## Best Practices

1. **Always test on TestPyPI first**
2. **Use semantic versioning** (MAJOR.MINOR.PATCH)
3. **Keep a CHANGELOG** documenting changes
4. **Tag releases in Git** matching PyPI versions
5. **Use API tokens** instead of passwords
6. **Don't commit tokens** to version control
7. **Test installation** before announcing
8. **Update documentation** after publishing

## Security

### Protecting Your API Token

Never commit your API token! Use:

1. **Environment variables:**
   ```bash
   export TWINE_USERNAME=__token__
   export TWINE_PASSWORD=pypi-...
   python3 -m twine upload dist/*
   ```

2. **`.pypirc` file** (in home directory):
   ```ini
   [pypi]
   username = __token__
   password = pypi-...
   
   [testpypi]
   username = __token__
   password = pypi-...
   ```
   
   Then:
   ```bash
   chmod 600 ~/.pypirc  # Protect the file
   ```

3. **GitHub Secrets** for automated publishing

## Post-Publishing

After successful publication:

1. **Announce** on relevant channels (Twitter, Reddit, etc.)
2. **Monitor** PyPI download stats
3. **Respond** to issues and questions
4. **Plan** next version based on feedback
5. **Update** documentation as needed

## Resources

- PyPI Help: https://pypi.org/help/
- Packaging Guide: https://packaging.python.org/
- Twine Documentation: https://twine.readthedocs.io/
- TestPyPI: https://test.pypi.org/

## Quick Reference

```bash
# Complete publishing workflow
rm -rf dist/ build/ *.egg-info/
python3 -m build
python3 -m twine check dist/*
python3 -m twine upload --repository testpypi dist/*  # Test first
python3 -m twine upload dist/*  # Then production
```
