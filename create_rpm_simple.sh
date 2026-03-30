#!/bin/bash
#
# create_rpm_simple.sh - Create RPM from installed files
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

PKG_NAME="facetonesoftphone"
PKG_VERSION="1.0.0"
PKG_RELEASE="1"
PKG_ARCH="x86_64"

echo "=================================================="
echo "  Creating FacetoneSoftPhone RPM Package"
echo "=================================================="

# Create temporary directory
TMP_DIR=$(mktemp -d)
INSTALL_ROOT="$TMP_DIR/root"
mkdir -p "$INSTALL_ROOT"

echo "[1/5] Collecting installed files..."

# Copy all installed files
mkdir -p "$INSTALL_ROOT/usr/local/bin"
cp -v /usr/local/bin/facetonesoftphone "$INSTALL_ROOT/usr/local/bin/"

mkdir -p "$INSTALL_ROOT/usr/local/lib64"
for lib in libbcunit.a libbctoolbox.a libbelr.a libbelcard.a libortp.a \
           libbelle-sip.a libmediastreamer2.a liblinphone.a; do
    [ -f "/usr/local/lib64/$lib" ] && cp -v "/usr/local/lib64/$lib" "$INSTALL_ROOT/usr/local/lib64/"
done

mkdir -p "$INSTALL_ROOT/usr/local/lib64/pkgconfig"
for pc in bcunit.pc bctoolbox.pc belr.pc belle-sip.pc ortp.pc \
          mediastreamer2.pc linphone.pc; do
    [ -f "/usr/local/lib64/pkgconfig/$pc" ] && cp -v "/usr/local/lib64/pkgconfig/$pc" "$INSTALL_ROOT/usr/local/lib64/pkgconfig/"
done

mkdir -p "$INSTALL_ROOT/usr/local/include"
for hdr in bctoolbox belr belcard ortp belle-sip mediastreamer2 linphone; do
    [ -d "/usr/local/include/$hdr" ] && cp -rv "/usr/local/include/$hdr" "$INSTALL_ROOT/usr/local/include/"
done

# Copy grammar files (CRITICAL for runtime!)
echo "Copying runtime data files..."
if [ -d "/usr/local/share/belr" ]; then
    mkdir -p "$INSTALL_ROOT/usr/local/share"
    cp -rv "/usr/local/share/belr" "$INSTALL_ROOT/usr/local/share/"
    echo "✓ Copied belr grammar files"
fi

if [ -d "/usr/local/share/linphone" ]; then
    mkdir -p "$INSTALL_ROOT/usr/local/share"
    cp -rv "/usr/local/share/linphone" "$INSTALL_ROOT/usr/local/share/"
    echo "✓ Copied linphone data files"
fi

mkdir -p "$INSTALL_ROOT/usr/local/share/facetonesoftphone"
cp -v ringtone.wav "$INSTALL_ROOT/usr/local/share/facetonesoftphone/"

mkdir -p "$INSTALL_ROOT/usr/local/share/applications"
cp -v LinuxSoftPhone.desktop "$INSTALL_ROOT/usr/local/share/applications/facetonesoftphone.desktop"

mkdir -p "$INSTALL_ROOT/usr/local/share/icons/hicolor/256x256/apps"
cp -v linuxsoftphone.png "$INSTALL_ROOT/usr/local/share/icons/hicolor/256x256/apps/facetonesoftphone.png"

mkdir -p "$INSTALL_ROOT/usr/local/share/doc/facetonesoftphone"
cp -v BUILDING_FEDORA.md "$INSTALL_ROOT/usr/local/share/doc/facetonesoftphone/"

echo ""
echo "[2/5] Creating tarball..."
cd "$TMP_DIR"
tar czf "${PKG_NAME}-${PKG_VERSION}.tar.gz" -C root .

echo ""
echo "[3/5] Creating RPM spec file..."
mkdir -p rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
mv "${PKG_NAME}-${PKG_VERSION}.tar.gz" rpmbuild/SOURCES/

cat > rpmbuild/SPECS/${PKG_NAME}.spec << 'EOFSPEC'
%global debug_package %{nil}

Name:           facetonesoftphone
Version:        1.0.0
Release:        1%{?dist}
Summary:        Qt6-based SIP softphone for Fedora 43
License:        MIT  
URL:            https://github.com/facetone
Source0:        %{name}-%{version}.tar.gz
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
FacetoneSoftPhone is a Qt6-based SIP softphone for Fedora 43.
Includes entire linphone stack built with compatibility patches.

Features: Audio calling, WebSocket API, auto-answer, configurable devices.
Note: Video disabled (FFmpeg 7.1), DB storage disabled (SOCI 4.x).

%prep
%setup -q -c

%install
mkdir -p %{buildroot}
cp -a * %{buildroot}/

%files
/usr/local/bin/facetonesoftphone
/usr/local/lib64/*.a
/usr/local/lib64/pkgconfig/*.pc
/usr/local/include/*
%dir /usr/local/share/belr
/usr/local/share/belr/*
%dir /usr/local/share/linphone
/usr/local/share/linphone/*
/usr/local/share/facetonesoftphone/
/usr/local/share/applications/facetonesoftphone.desktop
/usr/local/share/icons/hicolor/256x256/apps/facetonesoftphone.png
/usr/local/share/doc/facetonesoftphone/

%post
/bin/touch --no-create /usr/local/share/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create /usr/local/share/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache /usr/local/share/icons/hicolor &>/dev/null || :
fi
/usr/bin/update-desktop-database &> /dev/null || :

%changelog
* Sun Mar 30 2025 FaceTone <support@facetone.com> - 1.0.0-1
- Initial Fedora 43 package with compatibility patches
EOFSPEC

echo ""
echo "[4/5] Building RPM..."
cd "$TMP_DIR"
rpmbuild --define "_topdir $TMP_DIR/rpmbuild" -bb rpmbuild/SPECS/${PKG_NAME}.spec

echo ""
echo "[5/5] Copying RPM to current directory..."
RPM_FILE=$(find rpmbuild/RPMS -name "*.rpm" | head -1)
if [ -n "$RPM_FILE" ]; then
    cp -v "$RPM_FILE" "$SCRIPT_DIR/"
    RPM_NAME=$(basename "$RPM_FILE")
    
    echo ""
    echo "=================================================="
    echo "  RPM Package Created Successfully!"
    echo "=================================================="
    echo ""
    echo "Package: $SCRIPT_DIR/$RPM_NAME"
    echo "Size: $(du -h "$SCRIPT_DIR/$RPM_NAME" | cut -f1)"
    echo ""
    echo "To install:"
    echo "  sudo dnf install ./$RPM_NAME"
    echo ""
    echo "To verify:"
    echo "  rpm -qpl $RPM_NAME | head -20"
    echo "  rpm -qpi $RPM_NAME"
    echo ""
else
    echo "ERROR: RPM not found!"
    exit 1
fi

# Cleanup
cd "$SCRIPT_DIR"
rm -rf "$TMP_DIR"
