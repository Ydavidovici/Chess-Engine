# engines/myengine/checkout-build.sh
#!/usr/bin/env bash
set -euo pipefail

# Checkout a specific version tag and build it
# Usage: ./checkout-build.sh vMAJOR.MINOR.PATCH

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 vMAJOR.MINOR.PATCH" >&2
  exit 1
fi

TAG=$1

echo "→ Fetching tags and checking out $TAG..."
# Ensure we have all tags
git fetch --tags
# Detach HEAD at the tag
git checkout "$TAG"

echo "→ Building source at $TAG..."
# Invoke the standard build script
./build.sh

echo "✅ Build complete for $TAG"