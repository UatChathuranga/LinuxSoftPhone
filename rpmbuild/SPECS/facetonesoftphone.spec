Name:           facetonesoftphone
Version:        1.0.1
Release:        1%{?dist}
Summary:        Portable Linux Softphone with bundled dependencies

License:        GPLv3
URL:            https://github.com/chathuranga/LinuxSoftPhone
Source0:        %{name}-%{version}.tar.gz

# We bundle most dependencies, so we don't list them here to avoid conflicts
# But we might need some basic system libs
Requires:       glibc, libgcc, libstdc++, libX11, libxcb

%description
Portable Linux Softphone with bundled dependencies.
Includes Qt6 and liblinphone libraries.

%prep
# No source unpacking needed if we use the script to prepare the buildroot

%build
# Build is handled by the script

%install
# The script will populate the buildroot
# Here we just list what needs to be moved to the final location
mkdir -p %{buildroot}/opt/%{name}
cp -r %{_sourcedir}/opt/%{name}/* %{buildroot}/opt/%{name}/

mkdir -p %{buildroot}/usr/bin
cp -r %{_sourcedir}/usr/bin/* %{buildroot}/usr/bin/

mkdir -p %{buildroot}/usr/share/applications
cp -r %{_sourcedir}/usr/share/applications/* %{buildroot}/usr/share/applications/

mkdir -p %{buildroot}/usr/share/icons/hicolor/256x256/apps
cp -r %{_sourcedir}/usr/share/icons/hicolor/256x256/apps/* %{buildroot}/usr/share/icons/hicolor/256x256/apps/

%post
/usr/bin/update-desktop-database &> /dev/null || :

%files
/opt/%{name}
/usr/bin/%{name}
/usr/share/applications/%{name}.desktop
/usr/share/icons/hicolor/256x256/apps/%{name}.png

%changelog
* Wed Mar 25 2026 Chathuranga <uatchathuranga@gmail.com> - 1.0.1-1
- Initial RPM release
