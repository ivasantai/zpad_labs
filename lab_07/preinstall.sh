#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

if command -v sudo >/dev/null 2>&1; then
    SUDO=sudo
else
    SUDO=
fi

if command -v apt >/dev/null 2>&1; then
    echo "Installing system dependencies..."
    if ! $SUDO apt update; then
        echo "Warning: apt update failed. Continuing because package indexes may already be available."
    fi
    $SUDO apt install -y build-essential cmake libopencv-dev curl
else
    echo "apt was not found. Skipping system package installation."
fi

mkdir -p models

PROTO_FILE="models/deploy.prototxt"
MODEL_FILE="models/res10_300x300_ssd_iter_140000.caffemodel"
PROTO_URL="https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt"
MODEL_URL="https://raw.githubusercontent.com/opencv/opencv_3rdparty/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel"

download_if_missing() {
    local url="$1"
    local output="$2"

    if [ -s "$output" ]; then
        echo "$output already exists."
        return 0
    fi

    echo "Downloading $output..."
    curl -L --fail --retry 3 --connect-timeout 20 "$url" -o "$output"
}

download_if_missing "$PROTO_URL" "$PROTO_FILE"
download_if_missing "$MODEL_URL" "$MODEL_FILE"

echo ""
echo "Dependencies and DNN model files are ready."
echo "Project directory: $PROJECT_DIR"
echo "Model files:"
echo "  $PROJECT_DIR/$PROTO_FILE"
echo "  $PROJECT_DIR/$MODEL_FILE"
