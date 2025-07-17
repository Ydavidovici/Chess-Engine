#!/usr/bin/env bash
set -euo pipefail

# Create and push a new version tag
# Usage: ./tag-release.sh MAJOR.MINOR.PATCH
if [[ $# -ne 1 ]]; then
  echo "Usage: $0 MAJOR.MINOR.PATCH" >&2
  exit 1
fi

VER=$1
TAG="v${VER}"

# Prompt for optional release notes
read -p "Release notes for ${TAG} (optional, press Enter to skip): " NOTES

git fetch origin
if [[ -n "$NOTES" ]]; then
  git tag -a "$TAG" -m "$NOTES"
else
  git tag "$TAG"
fi
# Push tag to remote
git push origin "$TAG"

echo "✅ Tagged and pushed ${TAG}"