#!/usr/bin/env bash
set -euo pipefail

TAG=${1:?"Usage: $0 <tag> (e.g. v1.2.3)"}
OUT="engines/myengine/${TAG}"
ASSET="release_${TAG}.tar.gz"

echo "=== Fetching ${TAG} ==="

# 1) Download the Release asset
gh release download "${TAG}" --pattern "${ASSET}" --dir .

# 2) Unpack into engines/myengine/<tag>
mkdir -p "${OUT}"
tar -xzf "${ASSET}" -C engines/myengine
rm "${ASSET}"

echo "✅ Extracted ${TAG} → ${OUT}/myengine_${TAG}"