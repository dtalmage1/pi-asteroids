#!/usr/bin/env bash
# Build and run the test suite on the RPi over SSH.
# Run from the project root or any subdirectory.
# Requires SSH key auth to dan@dtdan (no password prompt).
set -euo pipefail

REMOTE_HOST="dan@dtdan"
REMOTE_DIR="~/Documents/Projects/Asteroids"

echo "=== Building on RPi ==="
ssh "${REMOTE_HOST}" "cd ${REMOTE_DIR} && cmake -B build && cmake --build build"

echo "=== Running tests on RPi ==="
ssh "${REMOTE_HOST}" "cd ${REMOTE_DIR}/build && ctest --output-on-failure"

echo "=== Integration tests passed ==="
