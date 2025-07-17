# engines/myengine/list-versions.sh
#!/usr/bin/env bash
set -euo pipefail

# List all semantic‐version tags in descending order
# Usage: ./list-versions.sh

git fetch --tags
# Only show tags starting with 'v' followed by digits, sorted by semver descending
git tag --list "v[0-9]*.[0-9]*.[0-9]*" --sort=-v:refname