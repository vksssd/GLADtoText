#!/bin/bash

set -e

cd "$(dirname "$0")"

echo "Setting up GoogleTest..."
if [ ! -d "external/googletest" ]; then
    git clone --depth 1 --branch v1.14.0 https://github.com/google/googletest.git external/googletest
fi

echo "Creating build directory..."
mkdir -p build
cd build

echo "Running CMake..."
cmake ..

echo "Building..."
cmake --build .

echo "Running tests..."
ctest --output-on-failure

echo ""
echo "✓ All tests passed!"
