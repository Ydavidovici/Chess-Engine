#!/usr/bin/env bash
set -euo pipefail

# 1) Make sure we're on an exact tag
TAG=$(git describe --tags --exact-match 2>/dev/null) \
  || { echo "❌ No exact tag found.  Checkout a tag like 'git checkout v1.2.3'"; exit 1; }
VER=${TAG#v}   # strip the leading “v”

echo "=== Release Build for ${TAG} ==="

# 2) Clean & configure
rm -rf build
mkdir build && cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON -DVERSION="${VER}"
cmake --build . -- -j$(nproc)
cd ..

# 3) Install Python bindings into venv if present
if [ -n "${VIRTUAL_ENV:-}" ]; then
  echo "Detected virtualenv, installing chessengine…"
  PY_EXE="${VIRTUAL_ENV}/bin/python"
  [ -x "$PY_EXE" ] || PY_EXE=python3
  SITE=$("$PY_EXE" - <<'PYCODE'
import site
paths = getattr(site, "getsitepackages", site.getusersitepackages)()
print(paths[0])
PYCODE
  )
  cp build/chessengine*.so build/chessengine*.dylib "$SITE/" 2>/dev/null || true
  cp build/pyengine*.so    build/pyengine*.dylib    "$SITE/" 2>/dev/null || true
  echo "→ Installed into $SITE"
fi

# 4) Archive the engine binary
OUT_DIR="engines/myengine/${TAG}"
rm -rf "${OUT_DIR}" && mkdir -p "${OUT_DIR}"
cp build/myengine "${OUT_DIR}/myengine_${TAG}"
echo "→ Staged binary at ${OUT_DIR}/myengine_${TAG}"

# 5) Package & publish to GitHub Releases
ASSET="release_${TAG}.tar.gz"
tar -czf "${ASSET}" -C engines/myengine "${TAG}"
echo "→ Created package ${ASSET}"

# Create (or reuse) the Release and upload the asset
gh release create "${TAG}" \
  --title "Release ${TAG}" \
  --notes "Automated build for ${TAG}" \
  "${ASSET}" \
  || {
    # if release already existed, just upload the new asset
    gh release upload "${TAG}" "${ASSET}" --clobber
  }

echo "✅ Published ${TAG} to GitHub Releases"