#!/bin/bash

# Debian Packager for LinuxSoftPhone
# Bundles all dependencies into /opt/linuxsoftphone/lib

APP_NAME="facetonesoftphone"
VERSION="1.0.1"
PKG_DIR="${APP_NAME}_${VERSION}"
OPT_DIR="${PKG_DIR}/opt/${APP_NAME}"

echo "--- Building Project ---"
mkdir -p build && cd build
cmake ..
make -j$(nproc)
cd ..

echo "--- Cleaning old package directory ---"
rm -rf "$PKG_DIR"

echo "--- Creating Filesystem Structure ---"
mkdir -p "${OPT_DIR}/bin"
mkdir -p "${OPT_DIR}/lib/qt6/plugins"
mkdir -p "${OPT_DIR}/lib/mediastreamer/plugins"
mkdir -p "${OPT_DIR}/share"
mkdir -p "${PKG_DIR}/usr/bin"
mkdir -p "${PKG_DIR}/usr/share/applications"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/256x256/apps"
mkdir -p "${PKG_DIR}/DEBIAN"

echo "--- Copying Core Files ---"
cp build/LinuxSoftPhone "${OPT_DIR}/bin/LinuxSoftPhone"
chmod +x "${OPT_DIR}/bin/LinuxSoftPhone"
cp ringtone.wav "${OPT_DIR}/share/"
cp linuxsoftphone.png "${PKG_DIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png" || cp src/linuxsoftphone.png "${PKG_DIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
cp LinuxSoftPhone.desktop "${PKG_DIR}/usr/share/applications/${APP_NAME}.desktop"

echo "--- Copying Plugins ---"
# Copy Qt6 plugins
cp -r /usr/lib/x86_64-linux-gnu/qt6/plugins/* "${OPT_DIR}/lib/qt6/plugins/"
# Copy Mediastreamer plugins
cp -r /usr/lib/x86_64-linux-gnu/mediastreamer/plugins/* "${OPT_DIR}/lib/mediastreamer/plugins/"

echo "--- Bundling Resource Files (Grammars, CA Certs) ---"
# These are essential for liblinphone and belr to work on clean systems
mkdir -p "${OPT_DIR}/share/belr"
mkdir -p "${OPT_DIR}/share/linphone"
cp -r /usr/share/belr/* "${OPT_DIR}/share/belr/" 2>/dev/null || true
cp -r /usr/share/linphone/* "${OPT_DIR}/share/linphone/" 2>/dev/null || true

echo "--- Gathering Dependencies ---"
# Libraries to specifically include
LIBS=(
    "liblinphone.so"
    "libbctoolbox.so"
    "libbelr.so"
    "libbelcard.so"
    "libmediastreamer.so"
    "libortp.so"
    "libQt6"
)

# Common system libraries to EXCLUDE from bundling to avoid conflicts on other distros
EXCLUDE_PATTERN="(libc\.so|libm\.so|libpthread\.so|libdl\.so|librt\.so|libutil\.so|libresolv\.so|libgcc_s\.so|libstdc\+\+\.so|libX11\.so|libxcb\.so|libXau\.so|libXdmcp\.so|libXext\.so|libXrender\.so|libXi\.so|libXfixes\.so|libXcursor\.so|libXcomposite\.so|libXdamage\.so|libXrandr\.so|libXinerama\.so|libXft\.so|libglib-2\.0\.so|libgobject-2\.0\.so|libgthread-2\.0\.so|libgmodule-2\.0\.so|libgio-2\.0\.so|libdbus-1\.so|libsystemd\.so|libudev\.so|libcap\.so|libselinux\.so|libsepol\.so|libpam\.so|libaudit\.so|libapparmor\.so|libz\.so|liblzma\.so|libzstd\.so|liblz4\.so|libbz2\.so|libmount\.so|libblkid\.so|libuuid\.so|libasound\.so|libpulse\.so|libpulse-simple\.so|libpulse-mainloop-glib\.so|libjack\.so|libpipewire-0\.3\.so|libGL\.so|libvulkan\.so|libdrm\.so|libgbm\.so|libEGL\.so|libGLESv2\.so)"

for lib_prefix in "${LIBS[@]}"; do
    ldd build/LinuxSoftPhone | grep "$lib_prefix" | awk '{print $3}' | xargs -I {} cp -v {} "${OPT_DIR}/lib/"
done

# Ensure we have all transitive dependencies for the copied libs
find "${OPT_DIR}/lib/" -type f -name "*.so*" | xargs ldd | grep "=> /" | grep -E -v "$EXCLUDE_PATTERN" | awk '{print $3}' | xargs -I {} cp -u -v {} "${OPT_DIR}/lib/" 2>/dev/null

echo "--- Creating Launcher Wrapper ---"
cat <<EOF > "${PKG_DIR}/usr/bin/${APP_NAME}"
#!/bin/bash
export LD_LIBRARY_PATH=/opt/${APP_NAME}/lib:\$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=/opt/${APP_NAME}/lib/qt6/plugins
export MS_PLUGINS_DIR=/opt/${APP_NAME}/lib/mediastreamer/plugins
# Tell belr and liblinphone where to find their resources
export XDG_DATA_DIRS=/opt/${APP_NAME}/share:\$XDG_DATA_DIRS
export BELR_RESOURCES_DIR=/opt/${APP_NAME}/share/belr/grammars
export LINPHONE_RESOURCES_DIR=/opt/${APP_NAME}/share/linphone
exec /opt/${APP_NAME}/bin/LinuxSoftPhone "\$@"
EOF
chmod +x "${PKG_DIR}/usr/bin/${APP_NAME}"

echo "--- Creating Debian Metadata ---"
cat <<EOF > "${PKG_DIR}/DEBIAN/control"
Package: ${APP_NAME}
Version: ${VERSION}
Section: net
Priority: optional
Architecture: amd64
Maintainer: Chathuranga <uatchathuranga@gmail.com>
Description: Portable Linux Softphone with bundled dependencies.
 Includes Qt6 and liblinphone libraries.
EOF

cat <<EOF > "${PKG_DIR}/DEBIAN/postinst"
#!/bin/bash
update-desktop-database /usr/share/applications || true
# Create system log directory with permissive access
mkdir -p /var/log/facetonesoftphone
chmod 777 /var/log/facetonesoftphone
exit 0
EOF
chmod +x "${PKG_DIR}/DEBIAN/postinst"

echo "--- Building Debian Package ---"
dpkg-deb --build "$PKG_DIR"

echo "--- Done! ---"
if [ -f "${PKG_DIR}.deb" ]; then
    echo "Generated: ${PKG_DIR}.deb"
else
    echo "Error: Package generation failed."
fi
