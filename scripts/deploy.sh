#!/usr/bin/env bash
# Sync project source to the RPi. Run from the project root or any subdirectory.
# Requires rsync and SSH key auth to dan@dtdan (no password prompt).
# On Windows, run from Git Bash or any shell that has rsync on PATH.
set -euo pipefail

REMOTE_HOST="dan@dtdan"
REMOTE_DIR="~/Documents/Projects/Asteroids"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "Deploying ${PROJECT_ROOT} → ${REMOTE_HOST}:${REMOTE_DIR}"
rsync -avz --exclude='.git' --exclude='build/' \
    "${PROJECT_ROOT}/" \
    "${REMOTE_HOST}:${REMOTE_DIR}/"
echo "Deploy complete."
