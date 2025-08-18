#!/usr/bin/env bash
set -euo pipefail

echo "=== Cleaning previous build ==="
rm -rf build

# Prefer the venv's python if present
if [[ -n "${VIRTUAL_ENV:-}" && -x "$VIRTUAL_ENV/bin/python" ]]; then
  PY="$VIRTUAL_ENV/bin/python"
else
  PY="$(command -v python3 || command -v python)"
fi
echo "Using Python: $PY"

# Locate pybind11's CMake config dir from that Python
echo "=== Locating pybind11 CMake files via the active Python ==="
PYB11_DIR="$("$PY" - <<'PYCODE'
import sys
try:
    import pybind11
    sys.stdout.write(pybind11.get_cmake_dir())
except Exception as e:
    sys.stderr.write(f"ERROR: {e}\n")
    sys.exit(1)
PYCODE
)"
echo "pybind11 CMake dir: $PYB11_DIR"

echo "=== Configuring and building ==="
mkdir -p build
cmake -S . -B build \
  -DBUILD_PYTHON_BINDINGS=ON \
  -DPython3_EXECUTABLE="$PY" \
  -Dpybind11_DIR="$PYB11_DIR"

cmake --build build -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "=== Installing python module into venv (if any) ==="
if [ -n "${VIRTUAL_ENV:-}" ]; then
  echo "Detected virtualenv at: $VIRTUAL_ENV"
  PYTHON_EXE="$PY"

  # Use platlib for compiled extensions
  PY_SITE="$("$PYTHON_EXE" - <<'PYCODE'
import sysconfig
print(sysconfig.get_paths().get("platlib") or sysconfig.get_paths().get("purelib"))
PYCODE
)"
  echo "Installing into: $PY_SITE"

  # Copy any produced extension modules (adjust prefixes to match your targets)
  shopt -s nullglob
  cp build/*/chessengine*.so "$PY_SITE/" 2>/dev/null || true
  cp build/*/pyengine*.so    "$PY_SITE/" 2>/dev/null || true
  cp build/chessengine*.so   "$PY_SITE/" 2>/dev/null || true
  cp build/pyengine*.so      "$PY_SITE/" 2>/dev/null || true
  # macOS / Windows variants
  cp build/*/chessengine*.dylib "$PY_SITE/" 2>/dev/null || true
  cp build/*/pyengine*.dylib    "$PY_SITE/" 2>/dev/null || true
  cp build/*/chessengine*.pyd   "$PY_SITE/" 2>/dev/null || true
  cp build/*/pyengine*.pyd      "$PY_SITE/" 2>/dev/null || true
  shopt -u nullglob

  echo "Module installed. You should now be able to 'import chessengine' in this venv."
else
  echo "Not in a virtualenv; skipping python install."
fi

echo "=== Build + install complete ==="