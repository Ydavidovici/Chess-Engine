#!/usr/bin/env bash
set -euo pipefail

# Checkout a specific version tag and build it
# Usage: ./checkout-build.sh vMAJOR.MINOR.PATCH
if [[ $# -ne 1 ]]; then
  echo "Usage: $0 vMAJOR.MINOR.PATCH" >&2
  exit 1
fi

TAG=$1
echo "Checking out ${TAG}..."
git fetch --tags
git checkout "${TAG}"

echo "Building source at ${TAG}..."
./build.sh

echo "✅ Build complete for ${TAG}"