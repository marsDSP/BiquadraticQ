#!/bin/bash

# Define paths (CLI args override defaults)
SOURCE_BUNDLE="${1:-$HOME/CLionProjects/BiquadraticQ/cmake-build-debug/BiquadraticQ_artefacts/Debug/VST3/BiquadraticQ.vst3}"
DEST_DIR="${2:-$HOME/Desktop/vst test}"
VST3_NAME="$(basename "$SOURCE_BUNDLE")"

# Check if source file exists
if [ ! -d "$SOURCE_BUNDLE" ]; then
    echo "Error: Source VST3 not found at '$SOURCE_BUNDLE'"
    exit 1
fi

# Create destination directory if it doesn't exist
mkdir -p "$DEST_DIR"

DEST_BUNDLE="$DEST_DIR/$VST3_NAME"

# If a bundle with the same name already exists, delete it first for a clean replace
if [ -e "$DEST_BUNDLE" ]; then
    echo "Found existing $VST3_NAME at $DEST_DIR; deleting before copy..."
    rm -rf "$DEST_BUNDLE"

    if [ -e "$DEST_BUNDLE" ]; then
        echo "Error: Failed to remove existing bundle at '$DEST_BUNDLE'"
        exit 1
    fi
fi

# Copy the .vst3 bundle (it's a directory on macOS)
# Using -R to copy directories recursively
cp -R "$SOURCE_BUNDLE" "$DEST_DIR/"

if [ $? -eq 0 ]; then
    echo "Successfully copied $VST3_NAME to $DEST_DIR"
else
    echo "Failed to copy $VST3_NAME"
    exit 1
fi

# Copy assets alongside the VST3 bundle so the plugin can find IRs at runtime
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
ASSETS_SRC="$PROJECT_ROOT/assets"

if [ -d "$ASSETS_SRC/IRs" ]; then
    mkdir -p "$DEST_DIR/assets/IRs"
    rsync -a --delete "$ASSETS_SRC/IRs/" "$DEST_DIR/assets/IRs/"
    echo "Synced assets/IRs to $DEST_DIR/assets/IRs"
fi
