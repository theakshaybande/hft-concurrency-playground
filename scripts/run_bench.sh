#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" -j

echo "Running lock-free SPSC benchmark..."
"${BUILD_DIR}/bench_spsc" 2000000 65536

echo "Running mutex queue benchmark..."
"${BUILD_DIR}/bench_mutex_queue" 2000000
