# engines/myengine/tag-release.sh
#!/usr/bin/env bash
set -euo pipefail

# Create and push a new version tag
# Usage: ./tag-release.sh MAJOR.MINOR.PATCH

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 MAJOR.MINOR.PATCH" >&2
  exit 1
fi

VER=$1                # e.g. "1.2.3"
TAG="v${VER}"       # e.g. "v1.2.3"

# Optionally prompt for release notes
read -p "Release notes: " NOTES

git fetch origin
# Create annotated tag
git tag -a "$TAG" -m "$NOTES"
# Push tag to remote
git push origin "$TAG"

echo "✅ Tagged and pushed $TAG"