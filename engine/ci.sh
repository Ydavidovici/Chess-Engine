# ci.sh: full workflow: clean, build, test
#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=build
rm -rf "$BUILD_DIR"
echo "=== Cleaned $BUILD_DIR ==="

mkdir "$BUILD_DIR"
echo "=== Created $BUILD_DIR ==="

pushd "$BUILD_DIR" >/dev/null
cmake .. -DBUILD_PYTHON_BINDINGS=OFF
cmake --build .
ctest --output-on-failure
popd >/dev/null

echo "=== CI: build + test succeeded ==="