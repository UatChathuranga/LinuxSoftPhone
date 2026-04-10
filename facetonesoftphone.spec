Name:           facetonesoftphone
Version:        1.0.1
Release:        1%{?dist}
Summary:        Portable Linux Softphone with bundled dependencies

License:        GPLv3
URL:            https://github.com/chathuranga/LinuxSoftPhone
Source0:        %{name}-%{version}.tar.gz

%if 0%{?fedora}
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
%else
# Bundled approach (usually for builds on Ubuntu/Generic environments)
Requires:       glibc, libgcc, libstdc++, libX11, libxcb
%endif

%description
%if 0%{?fedora}
FacetoneSoftPhone is a modern Qt6-based SIP softphone application for Fedora Linux.
It provides audio calling capabilities using the linphone library stack.

Note: This package includes the entire linphone stack (bctoolbox, belr, belcard,
ortp, belle-sip, mediastreamer2, liblinphone) built with Fedora compatibility
patches.
%else
Portable Linux Softphone with bundled dependencies.
Includes Qt6 and liblinphone libraries.
%endif

%prep
%if 0%{?fedora}
%setup -q
%else
# No source unpacking needed for bundled approach
%endif

%build
%if 0%{?fedora}
# Build all linphone dependencies and FacetoneSoftPhone
./build_fedora.sh
%else
# Build is handled by external script for bundled approach
%endif

%install
rm -rf $RPM_BUILD_ROOT

%if 0%{?fedora}
# --- Fedora Native Install ---
# Install FacetoneSoftPhone
install -D -m 0755 build/facetonesoftphone %{buildroot}%{_bindir}/facetonesoftphone

# Install linphone libraries
mkdir -p %{buildroot}%{_libdir}
for lib in libbcunit.a libbctoolbox.a libbelr.a libbelcard.a libortp.a libbelle-sip.a libmediastreamer2.a liblinphone.a; do
    cp -a /usr/local/lib64/$lib %{buildroot}%{_libdir}/ 2>/dev/null || cp -a /usr/local/lib/$lib %{buildroot}%{_libdir}/ 2>/dev/null || true
done

# Install pkg-config files
mkdir -p %{buildroot}%{_libdir}/pkgconfig
cp -a /usr/local/lib64/pkgconfig/*.pc %{buildroot}%{_libdir}/pkgconfig/ 2>/dev/null || cp -a /usr/local/lib/pkgconfig/*.pc %{buildroot}%{_libdir}/pkgconfig/ 2>/dev/null || true

# Install headers
mkdir -p %{buildroot}%{_includedir}
for dir in bctoolbox belr belcard ortp belle-sip mediastreamer2 linphone; do
    cp -a /usr/local/include/$dir %{buildroot}%{_includedir}/ 2>/dev/null || true
done

# Install data files
install -D -m 0644 ringtone.wav %{buildroot}%{_datadir}/%{name}/ringtone.wav
install -D -m 0644 LinuxSoftPhone.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
install -D -m 0644 linuxsoftphone.png %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/%{name}.png

%else
# --- Bundled Generic Install ---
mkdir -p %{buildroot}/opt/%{name}
cp -r %{_sourcedir}/opt/%{name}/* %{buildroot}/opt/%{name}/

mkdir -p %{buildroot}/usr/bin
cp -r %{_sourcedir}/usr/bin/* %{buildroot}/usr/bin/

mkdir -p %{buildroot}/usr/share/applications
cp -r %{_sourcedir}/usr/share/applications/* %{buildroot}/usr/share/applications/

mkdir -p %{buildroot}/usr/share/icons/hicolor/256x256/apps
cp -r %{_sourcedir}/usr/share/icons/hicolor/256x256/apps/* %{buildroot}/usr/share/icons/hicolor/256x256/apps/
%endif

%post
# Update icon cache
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
# Update desktop database
/usr/bin/update-desktop-database &> /dev/null || :

# Create system log directory with permissive access
mkdir -p /var/log/facetonesoftphone
chmod 777 /var/log/facetonesoftphone

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
/usr/bin/update-desktop-database &> /dev/null || :

%files
%if 0%{?fedora}
%{_bindir}/facetonesoftphone
%{_libdir}/lib*.a
%{_libdir}/pkgconfig/*.pc
%{_includedir}/*
%{_datadir}/%{name}/
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png
%else
/opt/%{name}
/usr/bin/%{name}
/usr/share/applications/%{name}.desktop
/usr/share/icons/hicolor/256x256/apps/%{name}.png
%endif

%changelog
* Wed Mar 25 2026 Chathuranga <uatchathuranga@gmail.com> - 1.0.1-1
- Unified spec file supporting both Fedora native builds and bundled packaging
- Added /var/log/facetonesoftphone directory creation in %post
- Updated versions and metadata
