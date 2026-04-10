#!/bin/bash

# FacetoneSoftPhone Install Script
# This script installs dependencies, builds, and installs the application.

set -e

echo "--- FacetoneSoftPhone Installation ---"

# 1. Install Dependencies
echo "Step 1: Installing dependencies..."
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    qt6-base-dev \
    qt6-websockets-dev \
    liblinphone-dev \
    libpulse-dev \
    libv4l-dev \
    libglew-dev

# 2. Build the Project
echo "Step 2: Building the project..."
BUILD_DIR="build_install"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 3. Install to System
echo "Step 3: Installing to system..."
sudo make install

# 4. Post-installation steps
echo "Step 4: Post-installation configuration..."
# Ensure linphone data directory exists for the current user
mkdir -p "$HOME/.local/share/linphone"

# Update desktop database
sudo update-desktop-database /usr/local/share/applications || true

# Create system log directory
echo "Setting up application log directory..."
sudo mkdir -p /var/log/facetonesoftphone
sudo chmod 777 /var/log/facetonesoftphone

echo "--- Installation Complete! ---"
echo "You can now run the app by searching for 'FacetoneSoftPhone' in your application menu or by running 'facetonesoftphone' in the terminal."
