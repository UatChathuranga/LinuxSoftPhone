#!/bin/bash
#
# FacetoneSoftPhone Build Script for Fedora 43
# This script builds and installs all dependencies and the application
#
# Usage: sudo ./build_fedora.sh
#

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    log_error "Please run as root (use sudo)"
    exit 1
fi

# Get the actual user (when using sudo)
ACTUAL_USER="${SUDO_USER:-$USER}"
ACTUAL_HOME=$(getent passwd "$ACTUAL_USER" | cut -d: -f6)

log_info "Building FacetoneSoftPhone for Fedora 43"
log_info "Installing as user: $ACTUAL_USER"

# Step 1: Install system dependencies
log_info "Step 1: Installing system dependencies..."
dnf install -y \
    gcc gcc-c++ make cmake pkg-config git \
    qt6-qtbase-devel qt6-qtwebsockets-devel \
    alsa-lib-devel pulseaudio-libs-devel \
    speex-devel speexdsp-devel opus-devel \
    libvpx-devel x264-devel \
    sqlite-devel libxml2-devel zlib-devel \
    openssl-devel \
    glew-devel mesa-libGL-devel \
    gsm-devel \
    libjpeg-turbo-devel libpng-devel \
    turbojpeg-devel \
    gettext-devel \
    doxygen graphviz \
    libsrtp-devel \
    mbedtls-devel \
    ffmpeg-free-devel \
    xerces-c-devel \
    soci-devel soci-sqlite3-devel \
    zxing-cpp-devel \
    jsoncpp-devel

log_info "System dependencies installed successfully"

# Setup build environment
BUILD_DIR="/tmp/linphone_build_$$"
INSTALL_PREFIX="/usr/local"
PKG_CONFIG_PATH="$INSTALL_PREFIX/lib64/pkgconfig:$INSTALL_PREFIX/lib/pkgconfig:/usr/lib64/pkgconfig:/usr/share/pkgconfig"
export PKG_CONFIG_PATH

log_info "Build directory: $BUILD_DIR"
log_info "Install prefix: $INSTALL_PREFIX"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Function to build a cmake project
build_cmake_project() {
    local name=$1
    local repo=$2
    local branch=${3:-master}
    local extra_cmake_args=$4
    
    log_info "Building $name..."
    
    if [ -d "$name" ]; then
        log_warn "$name directory exists, removing..."
        rm -rf "$name"
    fi
    
    # Try to use cached copy first, then git clone
    if [ -d "/tmp/linphone_src_cache/$name" ]; then
        log_info "Using cached copy of $name"
        cp -r "/tmp/linphone_src_cache/$name" "$name"
    else
        log_info "Cloning $name from $repo"
        git clone --depth 1 -b "$branch" "$repo" "$name"
    fi
    
    cd "$name"
    
    mkdir -p build
    cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DCMAKE_PREFIX_PATH="$INSTALL_PREFIX" \
        -DCMAKE_C_FLAGS="-Wno-error=unused-parameter -Wno-error=maybe-uninitialized -Wno-error=unused-function" \
        -DCMAKE_CXX_FLAGS="-Wno-error=unused-parameter -Wno-error=maybe-uninitialized -Wno-error=unused-function" \
        -DENABLE_STATIC=OFF \
        -DENABLE_SHARED=ON \
        $extra_cmake_args
    
    make -j$(nproc)
    make install
    
    # Update ldconfig
    ldconfig
    
    cd "$BUILD_DIR"
    log_info "$name built and installed successfully"
}

# Step 2: Build BCUnit (required by bctoolbox)
build_cmake_project \
    "bcunit" \
    "https://gitlab.linphone.org/BC/public/bcunit.git" \
    "master" \
    "-DENABLE_TESTS=OFF"

# Step 3: Build bctoolbox (with mbedtls 3.x patch)
log_info "Building bctoolbox..."

if [ -d "bctoolbox" ]; then
    log_warn "bctoolbox directory exists, removing..."
    rm -rf "bctoolbox"
fi

# Try to use cached copy first
if [ -d "/tmp/linphone_src_cache/bctoolbox" ]; then
    log_info "Using cached copy of bctoolbox"
    cp -r "/tmp/linphone_src_cache/bctoolbox" "bctoolbox"
else
    git clone --depth 1 -b "master" "https://gitlab.linphone.org/BC/public/bctoolbox.git" "bctoolbox"
fi

cd "bctoolbox"

# Patch for mbedtls 3.x compatibility - comment out entire threading section
cat > /tmp/bctoolbox_mbedtls_patch.sed << 'EOF'
# Comment out the entire threading setup block (lines 80-85 approximately)
/mbedtls_threading_set_alt/,/threading_mutex_unlock_cpp);/{
    s/^/\/\/ /
}
# Comment out the threading free call (line 95 approximately)
/mbedtls_threading_free_alt/{
    s/^/\/\/ /
}
EOF

sed -i -f /tmp/bctoolbox_mbedtls_patch.sed src/crypto/mbedtls.cc

mkdir -p build
cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_PREFIX_PATH="$INSTALL_PREFIX" \
    -DCMAKE_C_FLAGS="-Wno-error=unused-parameter -Wno-error=maybe-uninitialized -Wno-error=unused-function" \
    -DCMAKE_CXX_FLAGS="-Wno-error=unused-parameter -Wno-error=maybe-uninitialized -Wno-error=unused-function" \
    -DENABLE_STATIC=OFF \
    -DENABLE_SHARED=ON \
    -DENABLE_TESTS=OFF -DENABLE_TESTS_COMPONENT=OFF -DENABLE_MBEDTLS=ON

make -j$(nproc)
make install

# Update ldconfig
ldconfig

cd "$BUILD_DIR"
log_info "bctoolbox built and installed successfully"

# Step 4: Build belr
build_cmake_project \
    "belr" \
    "https://gitlab.linphone.org/BC/public/belr.git" \
    "master" \
    "-DENABLE_TESTS=OFF -DENABLE_TESTS_COMPONENT=OFF -DENABLE_UNIT_TESTS=OFF -DENABLE_TOOLS=OFF"

# Step 5: Build belcard
build_cmake_project \
    "belcard" \
    "https://gitlab.linphone.org/BC/public/belcard.git" \
    "master" \
    "-DENABLE_TESTS=OFF -DENABLE_TESTS_COMPONENT=OFF -DENABLE_UNIT_TESTS=OFF -DENABLE_TOOLS=OFF"

# Step 6: Build ortp
build_cmake_project \
    "ortp" \
    "https://gitlab.linphone.org/BC/public/ortp.git" \
    "master" \
    "-DENABLE_TESTS=OFF -DENABLE_TESTS_COMPONENT=OFF -DENABLE_UNIT_TESTS=OFF"

# Step 7: Build belle-sip
build_cmake_project \
    "bellesip" \
    "https://gitlab.linphone.org/BC/public/belle-sip.git" \
    "master" \
    "-DENABLE_TESTS=OFF -DENABLE_TESTS_COMPONENT=OFF -DENABLE_UNIT_TESTS=OFF"

# Step 8: Build mediastreamer2
build_cmake_project \
    "mediastreamer2" \
    "https://gitlab.linphone.org/BC/public/mediastreamer2.git" \
    "master" \
    "-DENABLE_TESTS=OFF -DENABLE_PCAP=OFF -DENABLE_VIDEO=OFF -DENABLE_SOUND=ON -DENABLE_UNIT_TESTS=OFF -DENABLE_ZRTP=OFF"

# Step 9: Build linphone
build_cmake_project \
    "linphone-sdk" \
    "https://gitlab.linphone.org/BC/public/liblinphone.git" \
    "master" \
    "-DENABLE_TESTS=OFF -DENABLE_UNIT_TESTS=OFF -DENABLE_CONSOLE_UI=OFF -DENABLE_GTK_UI=OFF -DENABLE_CXX_WRAPPER=OFF -DENABLE_TOOLS=OFF -DENABLE_ZRTP=OFF -DENABLE_LIME=OFF -DENABLE_LIME_X3DH=OFF -DENABLE_DB_STORAGE=OFF -DENABLE_VIDEO=OFF"

# Step 9.5: Fix pkg-config files
log_info "Creating missing pkg-config files..."

# Fix belle-sip.pc library name
sed -i 's/-lbellesip/-lbelle-sip/g' "${INSTALL_PREFIX}/lib64/pkgconfig/belle-sip.pc"

# Create mediastreamer2.pc
cat > /tmp/mediastreamer2.pc << 'EOF'
prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib64
includedir=${prefix}/include

Name: Mediastreamer2
Description: Audio/video real-time streaming library
Version: 2.16.1
Requires: ortp
Libs: -L${libdir} -lmediastreamer2
Cflags: -I${includedir}/mediastreamer2
EOF
cp /tmp/mediastreamer2.pc "${INSTALL_PREFIX}/lib64/pkgconfig/mediastreamer2.pc"

# Create linphone.pc with all dependencies
cat > /tmp/linphone.pc << 'EOF'
prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib64
includedir=${prefix}/include

Name: linphone
Description: Linphone SIP library
Version: 5.0.0
Requires: belle-sip mediastreamer2 ortp bctoolbox
Libs: -L${libdir} -llinphone -lbelle-sip -lbelr -lbelcard -lz -lxml2 -lpthread -lm -ldl -lmbedtls -lmbedx509 -lmbedcrypto -lspeex -lspeexdsp -lopus -lgsm -lasound -lpulse -lsrtp2 -lssl -lcrypto -lsqlite3 -ljsoncpp -lxerces-c
Cflags: -I${includedir} -I${includedir}/linphone
EOF
cp /tmp/linphone.pc "${INSTALL_PREFIX}/lib64/pkgconfig/linphone.pc"

log_info "pkg-config files created successfully"

# Step 10: Build FacetoneSoftPhone
log_info "Building FacetoneSoftPhone..."

SOFTPHONE_DIR="/mnt/NyxOne/WORK/FaceTone/Facetone_Linux_Softphone/LinuxSoftPhone"
cd "$SOFTPHONE_DIR"

# Clean previous build
if [ -d "build" ]; then
    log_warn "Cleaning previous build directory..."
    rm -rf build
fi

mkdir -p build
cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"

make -j$(nproc)
make install

# Update desktop database
update-desktop-database /usr/local/share/applications 2>/dev/null || true

# Create linphone config directory for user
log_info "Creating linphone configuration directory..."
sudo -u "$ACTUAL_USER" mkdir -p "$ACTUAL_HOME/.local/share/linphone"

# Cleanup
log_info "Cleaning up build directory..."
cd /
rm -rf "$BUILD_DIR"

log_info "======================================"
log_info "Build completed successfully!"
log_info "======================================"
log_info "FacetoneSoftPhone is now installed."
log_info "Run it with: facetonesoftphone"
log_info "Or find it in your application menu."
log_info ""
log_info "PKG_CONFIG_PATH for future builds:"
log_info "export PKG_CONFIG_PATH=$PKG_CONFIG_PATH"

exit 0
