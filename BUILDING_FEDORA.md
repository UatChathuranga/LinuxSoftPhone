# Building FacetoneSoftPhone on Fedora 43

This document provides complete instructions for building FacetoneSoftPhone and all its dependencies on Fedora 43.

## Overview

FacetoneSoftPhone is a Qt6-based SIP softphone that requires the linphone SIP library stack. Since these libraries are not available in Fedora repositories, they must be built from source.

## System Requirements

- Fedora 43 (x86_64)
- Internet connection for downloading source code and dependencies
- Approximately 2GB free disk space
- Build time: 30-60 minutes

## Architecture

The build process consists of:

1. **System Dependencies** - Install development packages from Fedora repos
2. **Linphone Stack** - Build 7 interdependent libraries from source:
   - BCUnit (unit testing framework)
   - bctoolbox (base utility library)
   - belr (language recognition library)
   - belcard (vCard parser)
   - ortp (RTP protocol implementation)
   - mediastreamer2 (media streaming library)
   - liblinphone (SIP protocol library)
3. **FacetoneSoftPhone** - Build the application

## Dependency Chain

```
BCUnit (test framework)
  └─> bctoolbox (utilities + crypto)
        └─> belr (parser)
              └─> belcard (vCard)
        └─> ortp (RTP)
              └─> mediastreamer2 (media)
                    └─> liblinphone (SIP)
                          └─> FacetoneSoftPhone (application)
```

## Source Repositories

All linphone libraries are hosted at GitLab:
- **Base URL**: https://gitlab.linphone.org/BC/public/
- **Repositories**:
  - bcunit.git
  - bctoolbox.git  
  - belr.git
  - belcard.git
  - ortp.git
  - mediastreamer2.git
  - liblinphone.git

## System Dependencies

### Required Fedora Packages

```bash
sudo dnf install -y \
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
    gettext-devel \
    doxygen graphviz \
    libsrtp-devel \
    mbedtls-devel \
    ffmpeg-free-devel
```

### Package Purpose

| Package | Purpose |
|---------|---------|
| gcc, gcc-c++, make, cmake | Build toolchain |
| pkg-config | Library discovery |
| qt6-* | GUI framework |
| alsa-lib, pulseaudio | Audio I/O |
| speex, opus | Audio codecs |
| libvpx, x264, ffmpeg | Video codecs |
| libsrtp | Secure RTP |
| openssl, mbedtls | Cryptography |
| sqlite, libxml2 | Data storage/parsing |

## Build Process

### Automated Build Script

Use the provided `build_fedora.sh` script:

```bash
cd /path/to/FacetoneSoftPhone
sudo ./build_fedora.sh
```

The script:
- Installs system dependencies
- Downloads and builds all linphone libraries in correct order
- Builds and installs FacetoneSoftPhone
- Configures desktop integration

### Build Configuration

All libraries are built with:
- **Install prefix**: `/usr/local`
- **Build type**: Release
- **PKG_CONFIG_PATH**: `/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig`
- **Tests**: Disabled (to avoid extra dependencies)
- **ZRTP**: Disabled (encryption feature with problematic dependencies)

### Manual Build Steps

If you need to build manually:

```bash
export PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
BUILD_DIR="/tmp/linphone_build"
mkdir -p "$BUILD_DIR"

# 1. BCUnit
cd "$BUILD_DIR"
git clone https://gitlab.linphone.org/BC/public/bcunit.git
cd bcunit && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc) && sudo make install

# 2. bctoolbox (with mbedtls 3.x patch)
cd "$BUILD_DIR"
git clone https://gitlab.linphone.org/BC/public/bctoolbox.git
cd bctoolbox
# Apply patch for mbedtls 3.6.x compatibility
sed -i 's/mbedtls_threading_set_alt/\/\/ mbedtls_threading_set_alt/g' src/crypto/mbedtls.cc
sed -i 's/mbedtls_threading_free_alt/\/\/ mbedtls_threading_free_alt/g' src/crypto/mbedtls.cc
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_MBEDTLS=ON \
    -DENABLE_TESTS=OFF
make -j$(nproc) && sudo make install && sudo ldconfig

# 3. belr
cd "$BUILD_DIR"
git clone https://gitlab.linphone.org/BC/public/belr.git
cd belr && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_TOOLS=OFF -DENABLE_TESTS=OFF
make -j$(nproc) && sudo make install

# 4. belcard
cd "$BUILD_DIR"
git clone https://gitlab.linphone.org/BC/public/belcard.git
cd belcard && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_TOOLS=OFF -DENABLE_TESTS=OFF
make -j$(nproc) && sudo make install

# 5. ortp
cd "$BUILD_DIR"
git clone https://gitlab.linphone.org/BC/public/ortp.git
cd ortp && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_C_FLAGS="-Wno-error=maybe-uninitialized" \
    -DENABLE_TESTS=OFF
make -j$(nproc) && sudo make install && sudo ldconfig

# 6. mediastreamer2
cd "$BUILD_DIR"
git clone https://gitlab.linphone.org/BC/public/mediastreamer2.git
cd mediastreamer2 && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_VIDEO=ON \
    -DENABLE_SOUND=ON \
    -DENABLE_ZRTP=OFF \
    -DENABLE_TESTS=OFF
make -j$(nproc) && sudo make install && sudo ldconfig

# 7. liblinphone
cd "$BUILD_DIR"
git clone https://gitlab.linphone.org/BC/public/liblinphone.git
cd liblinphone && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_CONSOLE_UI=OFF \
    -DENABLE_GTK_UI=OFF \
    -DENABLE_ZRTP=OFF \
    -DENABLE_TESTS=OFF
make -j$(nproc) && sudo make install && sudo ldconfig

# 8. FacetoneSoftPhone
cd /path/to/FacetoneSoftPhone
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc) && sudo make install
sudo update-desktop-database
```

## Known Issues & Solutions

### Issue 1: mbedtls 3.x Threading API Changes

**Problem**: Fedora 43 ships mbedtls 3.6.5, which removed `mbedtls_threading_*` functions.

**Solution**: Patch bctoolbox to comment out threading initialization code (not needed for single-threaded use).

### Issue 2: Compiler Warnings as Errors

**Problem**: GCC 15.2 in Fedora 43 is strict about warnings.

**Solution**: Add `-Wno-error=unused-parameter -Wno-error=maybe-uninitialized -Wno-error=unused-function` to CFLAGS/CXXFLAGS.

### Issue 3: Network Timeouts

**Problem**: git.linphone.org may have intermittent connectivity.

**Solution**: Use retry loops or download source archives in advance.

### Issue 4: Missing ZRTP/Crypto Features

**Problem**: BZRTP requires bctoolbox with full crypto support, which conflicts with mbedtls 3.x.

**Solution**: Disable ZRTP (-DENABLE_ZRTP=OFF) - not essential for basic softphone functionality.

## Verification

After successful build:

```bash
# Check installation
which facetonesoftphone
# Should return: /usr/local/bin/facetonesoftphone

# Check library linkage
ldd /usr/local/bin/facetonesoftphone | grep linphone
# Should show liblinphone.so found in /usr/local/lib64

# Test linphone config directory
mkdir -p ~/.local/share/linphone

# Launch application
facetonesoftphone
```

## File Locations

- **Binary**: `/usr/local/bin/facetonesoftphone`
- **Libraries**: `/usr/local/lib64/lib*.so`
- **Headers**: `/usr/local/include/`
- **PKG-Config**: `/usr/local/lib64/pkgconfig/*.pc`
- **Desktop Entry**: `/usr/local/share/applications/facetonesoftphone.desktop`
- **Icon**: `/usr/local/share/icons/hicolor/256x256/apps/facetonesoftphone.png`
- **Ringtone**: `/usr/local/share/facetonesoftphone/ringtone.wav`
- **User Config**: `~/.local/share/linphone/`

## Uninstallation

To remove FacetoneSoftPhone and linphone libraries:

```bash
sudo rm -f /usr/local/bin/facetonesoftphone
sudo rm -f /usr/local/lib64/lib{linphone,mediastreamer,ortp,belcard,belr,bctoolbox,bcunit}.*
sudo rm -rf /usr/local/include/{linphone,mediastreamer2,ortp,belcard,belr,bctoolbox,BCUnit}
sudo rm -rf /usr/local/share/{applications/facetonesoftphone.desktop,facetonesoftphone,icons/hicolor/*/apps/facetonesoftphone.png}
sudo rm -rf /usr/local/share/{Linphone,Mediastreamer2,Ortp,Belcard,Belr,BCToolbox,BCUnit}
sudo rm -f /usr/local/lib64/pkgconfig/{linphone,mediastreamer,ortp,belcard,belr,bctoolbox,bcunit}.pc
sudo ldconfig
sudo update-desktop-database
```

## Troubleshooting

### Build Fails at Specific Library

1. Check `/tmp/linphone_build_*/` for error logs
2. Verify PKG_CONFIG_PATH includes `/usr/local/lib64/pkgconfig`
3. Ensure previous libraries installed correctly: `pkg-config --exists <libname>`
4. Check compiler flags for the failing library

### Application Won't Start

1. Check library dependencies: `ldd /usr/local/bin/facetonesoftphone`
2. Verify LD_LIBRARY_PATH or add `/usr/local/lib64` to `/etc/ld.so.conf.d/`
3. Run `sudo ldconfig` to update library cache
4. Check for Qt6 plugin errors with `QT_DEBUG_PLUGINS=1 facetonesoftphone`

### Audio/Video Not Working

1. Test audio devices: `arecord -l` and `aplay -l`
2. Check PulseAudio: `pactl info`
3. Verify codec libraries are installed
4. Check linphone config in `~/.local/share/linphone/`

## Building on Other Fedora Versions

For Fedora < 43:
- Adjust package names (qt6 might be qt5)
- May need older FFmpeg or video codec packages
- GCC warnings might differ

For Fedora > 43:
- Should work with minimal changes
- Watch for API changes in system libraries (mbedtls, ffmpeg)

## References

- **Linphone Project**: https://www.linphone.org/
- **Linphone GitLab**: https://gitlab.linphone.org/BC/public
- **Linphone Documentation**: https://www.linphone.org/technical-corner/liblinphone
- **Qt6 Documentation**: https://doc.qt.io/qt-6/
- **Fedora Documentation**: https://docs.fedoraproject.org/

## Support & Contributing

For issues specific to FacetoneSoftPhone, check the project repository.
For linphone library issues, consult https://gitlab.linphone.org/BC/public/liblinphone

---

*Document Version*: 1.0  
*Last Updated*: 2026-03-27  
*Target Platform*: Fedora 43 (x86_64)
