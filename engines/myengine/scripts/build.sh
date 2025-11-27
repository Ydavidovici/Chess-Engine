#!/usr/bin/env bash
set -euo pipefail

echo "=== Cleaning previous build ==="
rm -rf build

BUILD_TYPE=${1:-Release}

echo "=== Configuring (CMAKE_BUILD_TYPE=${BUILD_TYPE}) ==="
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo "=== Building ==="
cmake --build build -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "=== Done ==="