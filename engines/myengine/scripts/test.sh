#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   scripts/test.sh                # build (if needed) + run ALL tests
#   scripts/test.sh tests/myengine/board-move-test.cpp
#   scripts/test.sh board_move_tests
#   scripts/test.sh '^engine_.*moves$'      # direct regex

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="$repo_root/build"

# Ensure build exists & is configured
if [[ ! -f "$build_dir/CMakeCache.txt" ]]; then
  echo "=== Configuring CMake (first run) ==="
  cmake -S "$repo_root" -B "$build_dir" -DCMAKE_BUILD_TYPE=RelWithDebInfo
fi

# Always build before test (fast when up to date)
echo "=== Building ==="
cmake --build "$build_dir" -j"$(command -v nproc >/dev/null && nproc || sysctl -n hw.logicalcpu || echo 4)"

echo "=== Running tests ==="
cd "$build_dir"

# If an argument is provided, try to filter tests by filename stem (or accept a raw regex)
if [[ $# -ge 1 ]]; then
  arg="$1"
  if [[ "$arg" == *.cpp || "$arg" == *.cc || "$arg" == *.cxx || "$arg" == *.C ]]; then
    # Extract stem (basename without extension) for -R
    fname="$(basename "$arg")"
    stem="${fname%.*}"
    regex="$stem"
  else
    # Treat the arg as a regex directly
    regex="$arg"
  fi

  echo "→ Filtering tests with regex: $regex"
  # Show what would run (dry-run)
  echo "=== Matching tests (dry-run) ==="
  ctest -N -R "$regex" || true
  echo "=== Running matching tests ==="
  ctest -R "$regex" --output-on-failure
else
  # No filter → run everything
  ctest --output-on-failure
fi
