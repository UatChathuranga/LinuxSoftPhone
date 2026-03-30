Name:           facetonesoftphone
Version:        1.0.0
Release:        1%{?dist}
Summary:        Qt6-based SIP softphone for Fedora

License:        MIT
URL:            https://github.com/facetone/FacetoneSoftPhone
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  cmake >= 3.10
BuildRequires:  make
BuildRequires:  pkg-config
BuildRequires:  git
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtwebsockets-devel
BuildRequires:  alsa-lib-devel
BuildRequires:  pulseaudio-libs-devel
BuildRequires:  speex-devel
BuildRequires:  speexdsp-devel
BuildRequires:  opus-devel
BuildRequires:  sqlite-devel
BuildRequires:  libxml2-devel
BuildRequires:  zlib-devel
BuildRequires:  openssl-devel
BuildRequires:  gsm-devel
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  turbojpeg-devel
BuildRequires:  gettext-devel
BuildRequires:  libsrtp-devel
BuildRequires:  mbedtls-devel
BuildRequires:  xerces-c-devel
BuildRequires:  jsoncpp-devel

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
- Audio-only SIP calling
- WebSocket API for integration
- Auto-answer support
- Configurable audio devices
- Built specifically for Fedora 43

Note: This package includes the entire linphone stack (bctoolbox, belr, belcard,
ortp, belle-sip, mediastreamer2, liblinphone) built with Fedora 43 compatibility
patches.

%prep
%setup -q

%build
# Build all linphone dependencies and FacetoneSoftPhone
./build_fedora.sh

%install
rm -rf $RPM_BUILD_ROOT

# Install FacetoneSoftPhone
install -D -m 0755 build/facetonesoftphone %{buildroot}%{_bindir}/facetonesoftphone

# Install linphone libraries
mkdir -p %{buildroot}%{_libdir}
cp -a /usr/local/lib64/libbcunit.a %{buildroot}%{_libdir}/
cp -a /usr/local/lib64/libbctoolbox.a %{buildroot}%{_libdir}/
cp -a /usr/local/lib64/libbelr.a %{buildroot}%{_libdir}/
cp -a /usr/local/lib64/libbelcard.a %{buildroot}%{_libdir}/
cp -a /usr/local/lib64/libortp.a %{buildroot}%{_libdir}/
cp -a /usr/local/lib64/libbelle-sip.a %{buildroot}%{_libdir}/
cp -a /usr/local/lib64/libmediastreamer2.a %{buildroot}%{_libdir}/
cp -a /usr/local/lib64/liblinphone.a %{buildroot}%{_libdir}/

# Install pkg-config files
mkdir -p %{buildroot}%{_libdir}/pkgconfig
cp -a /usr/local/lib64/pkgconfig/*.pc %{buildroot}%{_libdir}/pkgconfig/

# Install headers
mkdir -p %{buildroot}%{_includedir}
cp -a /usr/local/include/bctoolbox %{buildroot}%{_includedir}/ 2>/dev/null || true
cp -a /usr/local/include/belr %{buildroot}%{_includedir}/ 2>/dev/null || true
cp -a /usr/local/include/belcard %{buildroot}%{_includedir}/ 2>/dev/null || true
cp -a /usr/local/include/ortp %{buildroot}%{_includedir}/ 2>/dev/null || true
cp -a /usr/local/include/belle-sip %{buildroot}%{_includedir}/ 2>/dev/null || true
cp -a /usr/local/include/mediastreamer2 %{buildroot}%{_includedir}/ 2>/dev/null || true
cp -a /usr/local/include/linphone %{buildroot}%{_includedir}/ 2>/dev/null || true

# Install data files
install -D -m 0644 ringtone.wav %{buildroot}%{_datadir}/%{name}/ringtone.wav
install -D -m 0644 LinuxSoftPhone.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
install -D -m 0644 linuxsoftphone.png %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/%{name}.png

# Install documentation
install -D -m 0644 BUILDING_FEDORA.md %{buildroot}%{_docdir}/%{name}/BUILDING_FEDORA.md

%files
%{_bindir}/facetonesoftphone
%{_libdir}/libbcunit.a
%{_libdir}/libbctoolbox.a
%{_libdir}/libbelr.a
%{_libdir}/libbelcard.a
%{_libdir}/libortp.a
%{_libdir}/libbelle-sip.a
%{_libdir}/libmediastreamer2.a
%{_libdir}/liblinphone.a
%{_libdir}/pkgconfig/*.pc
%{_includedir}/bctoolbox
%{_includedir}/belr
%{_includedir}/belcard
%{_includedir}/ortp
%{_includedir}/belle-sip
%{_includedir}/mediastreamer2
%{_includedir}/linphone
%{_datadir}/%{name}/
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png
%{_docdir}/%{name}/

%post
# Update icon cache
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
# Update desktop database
/usr/bin/update-desktop-database &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
/usr/bin/update-desktop-database &> /dev/null || :

%changelog
* Sun Mar 30 2026 FaceTone Team <support@facetone.com> - 1.0.0-1
- Initial RPM package for Fedora 43
- Built with Fedora 43 compatibility patches
- Audio-only support (video disabled due to FFmpeg 7.1)
- Database storage disabled (SOCI 4.x compatibility)
- Includes entire linphone stack with mbedtls 3.x patches
