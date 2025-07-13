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
  # Figure out where this venv’s site-packages lives
  PY_SITE=$(python - <<'PYCODE'
import site, sys
# site.getsitepackages() may not work in all venvs; fallback to user_site
paths = site.getsitepackages() if hasattr(site, "getsitepackages") else []
if not paths:
    paths = [site.getusersitepackages()]
print(paths[0])
PYCODE
)
  echo "Detected venv site-packages at: $PY_SITE"

  # Copy any engine module (wildcard covers .so/.dylib)
  echo "Copying module files to site-packages..."
  cp chessengine*.so chessengine*.dylib "$PY_SITE/" 2>/dev/null || true
  cp pyengine*.so pyengine*.dylib "$PY_SITE/" 2>/dev/null || true

  echo "Module installed. You should now be able to 'import chessengine' anywhere in this venv."
else
  echo "Not in a virtualenv; skipping python install."
fi

echo "=== Build + install complete ==="