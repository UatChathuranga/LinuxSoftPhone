#!/bin/bash

# Configuration
BUILD_DIR="build"
PROJECT_ROOT=$(pwd)

echo "--- Building FacetoneSoftPhone ---"

# 1. Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# 2. Check for dependencies
if [ ! -d "/usr/lib/x86_64-linux-gnu/cmake/Qt6WebSockets" ]; then
    echo "Error: Qt6WebSockets is not installed."
    echo "Please run: sudo apt install qt6-websockets-dev"
    exit 1
fi

# 3. Build the project
cd "$BUILD_DIR" || exit 1
cmake ..
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "--- Build successful! ---"

# 3. Handle missing linphone database directory to prevent runtime error
# Linphone expects ~/.local/share/linphone/ to exist
LINPHONE_DATA_DIR="$HOME/.local/share/linphone"
if [ ! -d "$LINPHONE_DATA_DIR" ]; then
    echo "Creating missing liblinphone directory: $LINPHONE_DATA_DIR"
    mkdir -p "$LINPHONE_DATA_DIR"
fi

# 4. Run the application
echo "--- Running FacetoneSoftPhone ---"
./facetonesoftphone
