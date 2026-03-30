#!/bin/bash
#
# create_rpm.sh - Create a binary RPM package of FacetoneSoftPhone
#
# This script packages the already-built and installed FacetoneSoftPhone
# along with all its dependencies into an RPM that can be shared with
# other Fedora 43 users.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Package information
PKG_NAME="facetonesoftphone"
PKG_VERSION="1.0.0"
PKG_RELEASE="1.fc43"
PKG_ARCH="x86_64"

echo "=================================================="
echo "  Creating FacetoneSoftPhone RPM Package"
echo "=================================================="
echo "Package: ${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm"
echo ""

# Create RPM build directory structure
RPM_ROOT="${SCRIPT_DIR}/rpm-package"
BUILD_ROOT="${RPM_ROOT}/BUILDROOT"
RPMS_DIR="${RPM_ROOT}/RPMS"
SPECS_DIR="${RPM_ROOT}/SPECS"

rm -rf "$RPM_ROOT"
mkdir -p "$BUILD_ROOT" "$RPMS_DIR" "$SPECS_DIR"

# Create the installation root
INSTALL_ROOT="${BUILD_ROOT}"
mkdir -p "$INSTALL_ROOT"

echo "[1/6] Collecting installed files..."

# Copy binary
mkdir -p "$INSTALL_ROOT/usr/local/bin"
cp -v /usr/local/bin/facetonesoftphone "$INSTALL_ROOT/usr/local/bin/"

# Copy libraries
mkdir -p "$INSTALL_ROOT/usr/local/lib64"
for lib in libbcunit.a libbctoolbox.a libbelr.a libbelcard.a libortp.a \
           libbelle-sip.a libmediastreamer2.a liblinphone.a; do
    if [ -f "/usr/local/lib64/$lib" ]; then
        cp -v "/usr/local/lib64/$lib" "$INSTALL_ROOT/usr/local/lib64/"
    fi
done

# Copy pkg-config files
mkdir -p "$INSTALL_ROOT/usr/local/lib64/pkgconfig"
for pc in bcunit.pc bctoolbox.pc belr.pc belcard.pc ortp.pc belle-sip.pc \
          mediastreamer2.pc linphone.pc; do
    if [ -f "/usr/local/lib64/pkgconfig/$pc" ]; then
        cp -v "/usr/local/lib64/pkgconfig/$pc" "$INSTALL_ROOT/usr/local/lib64/pkgconfig/"
    fi
done

# Copy include headers
mkdir -p "$INSTALL_ROOT/usr/local/include"
for hdr in bctoolbox belr belcard ortp belle-sip mediastreamer2 linphone; do
    if [ -d "/usr/local/include/$hdr" ]; then
        cp -rv "/usr/local/include/$hdr" "$INSTALL_ROOT/usr/local/include/"
    fi
done

# Copy data files
mkdir -p "$INSTALL_ROOT/usr/local/share/facetonesoftphone"
cp -v ringtone.wav "$INSTALL_ROOT/usr/local/share/facetonesoftphone/"

mkdir -p "$INSTALL_ROOT/usr/local/share/applications"
cp -v LinuxSoftPhone.desktop "$INSTALL_ROOT/usr/local/share/applications/facetonesoftphone.desktop"

mkdir -p "$INSTALL_ROOT/usr/local/share/icons/hicolor/256x256/apps"
cp -v linuxsoftphone.png "$INSTALL_ROOT/usr/local/share/icons/hicolor/256x256/apps/facetonesoftphone.png"

# Copy documentation
mkdir -p "$INSTALL_ROOT/usr/local/share/doc/facetonesoftphone"
cp -v BUILDING_FEDORA.md "$INSTALL_ROOT/usr/local/share/doc/facetonesoftphone/"

echo ""
echo "[2/6] Creating RPM spec file..."

# Create a simple spec file for binary RPM
cat > "$SPECS_DIR/${PKG_NAME}.spec" << 'EOF'
Name:           facetonesoftphone
Version:        1.0.0
Release:        1%{?dist}
Summary:        Qt6-based SIP softphone for Fedora 43
License:        MIT
URL:            https://github.com/facetone/FacetoneSoftPhone
BuildArch:      x86_64

Requires:       qt6-qtbase
Requires:       qt6-qtwebsockets
Requires:       alsa-lib
Requires:       pulseaudio-libs
Requires:       speex
Requires:       speexdsp
Requires:       opus
Requires:       sqlite-libs
Requires:       libxml2
Requires:       zlib
Requires:       openssl-libs
Requires:       gsm
Requires:       libjpeg-turbo
Requires:       turbojpeg
Requires:       libsrtp
Requires:       mbedtls
Requires:       xerces-c
Requires:       jsoncpp

%description
FacetoneSoftPhone is a modern Qt6-based SIP softphone application for Fedora Linux.
It provides audio calling capabilities using the linphone library stack.

Features:
- Audio-only SIP calling (video disabled due to FFmpeg 7.1 compatibility)
- WebSocket API for integration
- Auto-answer support  
- Configurable audio devices
- Built specifically for Fedora 43 with compatibility patches

This package includes the entire linphone stack (bctoolbox, belr, belcard,
ortp, belle-sip, mediastreamer2, liblinphone) built with Fedora 43 compatibility
patches for mbedtls 3.x and GCC 15.2.

%install
# Files are already in BUILDROOT, nothing to do
true

%files
/usr/local/bin/facetonesoftphone
/usr/local/lib64/libbcunit.a
/usr/local/lib64/libbctoolbox.a
/usr/local/lib64/libbelr.a
/usr/local/lib64/libbelcard.a
/usr/local/lib64/libortp.a
/usr/local/lib64/libbelle-sip.a
/usr/local/lib64/libmediastreamer2.a
/usr/local/lib64/liblinphone.a
/usr/local/lib64/pkgconfig/*.pc
/usr/local/include/bctoolbox
/usr/local/include/belr
/usr/local/include/belcard
/usr/local/include/ortp
/usr/local/include/belle-sip
/usr/local/include/mediastreamer2
/usr/local/include/linphone
/usr/local/share/facetonesoftphone/
/usr/local/share/applications/facetonesoftphone.desktop
/usr/local/share/icons/hicolor/256x256/apps/facetonesoftphone.png
/usr/local/share/doc/facetonesoftphone/

%post
# Update icon cache
/bin/touch --no-create /usr/local/share/icons/hicolor &>/dev/null || :
# Update desktop database  
/usr/bin/update-desktop-database &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create /usr/local/share/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache /usr/local/share/icons/hicolor &>/dev/null || :
fi
/usr/bin/update-desktop-database &> /dev/null || :

%changelog
* Sun Mar 30 2026 FaceTone Team <support@facetone.com> - 1.0.0-1
- Initial RPM package for Fedora 43
- Built with Fedora 43 compatibility patches:
  * mbedtls 3.x API changes (threading functions removed)
  * FFmpeg 7.1 API changes (video disabled)
  * SOCI 4.x API changes (database storage disabled)
  * GCC 15.2 strict warnings handled
- Includes entire linphone stack pre-built
- Audio-only SIP calling support
EOF

echo "[3/6] Building RPM package..."

# Build the RPM
rpmbuild -bb \
    --define "_topdir $RPM_ROOT" \
    --define "_buildrootdir $BUILD_ROOT" \
    --define "_rpmdir $RPMS_DIR" \
    --define "_build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm" \
    --buildroot "$BUILD_ROOT" \
    "$SPECS_DIR/${PKG_NAME}.spec"

echo ""
echo "[4/6] Verifying RPM package..."
RPM_FILE="${RPMS_DIR}/${PKG_ARCH}/${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm"

if [ -f "$RPM_FILE" ]; then
    rpm -qpl "$RPM_FILE" | head -20
    echo "... (more files)"
    echo ""
    rpm -qpi "$RPM_FILE"
else
    echo "ERROR: RPM file not found at $RPM_FILE"
    exit 1
fi

echo ""
echo "[5/6] Testing RPM with rpmlint..."
rpmlint "$RPM_FILE" || true

echo ""
echo "[6/6] Copying RPM to current directory..."
cp -v "$RPM_FILE" "$SCRIPT_DIR/"

echo ""
echo "=================================================="
echo "  RPM Package Created Successfully!"
echo "=================================================="
echo ""
echo "Package location:"
echo "  ${SCRIPT_DIR}/${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm"
echo ""
echo "Package size:"
ls -lh "${SCRIPT_DIR}/${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm" | awk '{print "  " $5}'
echo ""
echo "To install on another Fedora 43 system:"
echo "  sudo dnf install ./${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm"
echo ""
echo "To share:"
echo "  scp ${PKG_NAME}-${PKG_VERSION}-${PKG_RELEASE}.${PKG_ARCH}.rpm user@host:"
echo ""
echo "Note: Recipients will need to install system dependencies first:"
echo "  sudo dnf install qt6-qtbase qt6-qtwebsockets alsa-lib pulseaudio-libs \\"
echo "    speex speexdsp opus sqlite-libs libxml2 zlib openssl-libs gsm \\"
echo "    libjpeg-turbo turbojpeg libsrtp mbedtls xerces-c jsoncpp"
echo ""
