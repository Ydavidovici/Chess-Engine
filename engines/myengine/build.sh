#!/usr/bin/env bash
set -euo pipefail

echo "=== Cleaning previous build ==="
rm -rf build

echo "=== Configuring and building ==="
mkdir -p build
cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON
cmake --build .

echo "=== Installing python module into venv (if any) ==="
if [ -n "${VIRTUAL_ENV:-}" ]; then
  echo "Detected virtualenv at: $VIRTUAL_ENV"

  # Use the venv’s python if available, otherwise fallback to python3
  PYTHON_EXE="${VIRTUAL_ENV}/bin/python"
  if ! [ -x "$PYTHON_EXE" ]; then
    PYTHON_EXE=python3
  fi

  # Find the venv’s site-packages directory
  PY_SITE=$("$PYTHON_EXE" - <<'PYCODE'
import site
paths = site.getsitepackages() if hasattr(site, "getsitepackages") else []
if not paths:
    paths = [site.getusersitepackages()]
print(paths[0])
PYCODE
  )
  echo "Installing into: $PY_SITE"

  # Copy the built extension modules into site-packages
  cp chessengine*.so chessengine*.dylib "$PY_SITE/" 2>/dev/null || true
  cp pyengine*.so    pyengine*.dylib    "$PY_SITE/" 2>/dev/null || true

  echo "Module installed. You should now be able to 'import chessengine' anywhere in this venv."
else
  echo "Not in a virtualenv; skipping python install."
fi

echo "=== Build + install complete ==="