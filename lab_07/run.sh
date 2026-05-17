#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

if [ ! -x "build/lab_07_app" ]; then
    echo "Executable build/lab_07_app was not found. Run ./build.sh first."
    exit 1
fi

./build/lab_07_app "$@"
