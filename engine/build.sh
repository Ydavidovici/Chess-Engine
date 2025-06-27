#!/usr/bin/env bash
set -euo pipefail

echo "=== Cleaning previous build ==="
rm -rf build

echo "=== Configuring and building ==="
mkdir -p build
cd build
cmake .. -DBUILD_PYTHON_BINDINGS=OFF
cmake --build .

echo "=== Build complete ==="
