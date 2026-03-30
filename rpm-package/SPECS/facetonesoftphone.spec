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
