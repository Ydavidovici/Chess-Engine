#!/usr/bin/env bash
set -euo pipefail

# Version-build script for engines/myengine
# Usage: ./version-build.sh MAJOR.MINOR.PATCH

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 MAJOR.MINOR.PATCH"
  exit 1
fi

VER=$1              # e.g. "1.2.3"
TAG="v${VER}"       # e.g. "v1.2.3"

SRC_DIR="build"     # build output directory under engines/myengine/
DEST_DIR="builds/${TAG}"   # new versioned directory under engines/myengine/

echo "→ Versioning 'myengine' build as ${TAG}…"

# 1) Ensure build output exists
if [[ ! -d "$SRC_DIR" ]]; then
  echo "❌ No build directory found at ${SRC_DIR}. Run your build first."
  exit 1
fi

# 2) Copy build/ → vX.Y.Z/
rm -rf "$DEST_DIR"
mkdir -p "$DEST_DIR"
cp -R "$SRC_DIR"/* "$DEST_DIR"/

# 3) (Optional) If you want to rename the binary:
#    mv "${DEST_DIR}/myengine" "${DEST_DIR}/myengine_${TAG}"

# 4) Stage only the new version folder
git add "$DEST_DIR"

# 5) Commit & tag
git commit -m "Add myengine build ${TAG}"

git tag -a "${TAG}" -m "Release ${TAG}"

# 6) Push commit and tag
git push origin HEAD
git push origin "${TAG}"

echo "✅ Build for ${TAG} staged in engines/myengine/${DEST_DIR}/ and tagged."