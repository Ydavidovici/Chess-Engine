```bash
#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 MAJOR.MINOR.PATCH"
  exit 1
fi

VER=$1                   # e.g. "1.2.3"
TAG="v${VER}"           # e.g. "v1.2.3"
SRC_DIR="build"         # your local build output under myengine/build
DEST_DIR="engines/myengine/${TAG}"  # versioned dir under myengine

echo "→ Versioning build of myengine as ${TAG} …"

# 1) Ensure build directory exists
if [[ ! -d "$SRC_DIR" ]]; then
  echo "❌ Build directory not found: $SRC_DIR"
  exit 1
fi

# 2) Copy artifacts into versioned folder
rm -rf "$DEST_DIR"
mkdir -p "$DEST_DIR"
cp -R "$SRC_DIR/"* "$DEST_DIR/"

# 3) Commit & tag
cd engines/myengine
# Add new build folder
git add "$TAG"
git commit -m "Add myengine build ${TAG}"
# Create annotated tag at this commit
git tag -a "${TAG}" -m "Release ${TAG}"
# Push both branch and tag
git push origin HEAD
git push origin "${TAG}"

echo "✅ myengine build staged in $DEST_DIR and tagged ${TAG}"
```