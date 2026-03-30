# FacetoneSoftPhone RPM Package for Fedora 43

## Package Information

- **Package Name**: `facetonesoftphone-1.0.0-1.fc43.x86_64.rpm`
- **Size**: ~12 MB
- **Architecture**: x86_64
- **Target OS**: Fedora 43

## What's Included

This RPM package contains:
- FacetoneSoftPhone application (Qt6-based SIP softphone)
- Complete linphone stack:
  - bctoolbox (with mbedtls 3.x compatibility patches)
  - belr, belcard (vCard support)
  - ortp (RTP protocol)
  - belle-sip (SIP parser)
  - mediastreamer2 (audio-only media streaming)
  - liblinphone (SIP library core)

## Features

- Audio-only SIP calling
- WebSocket API for integration
- Auto-answer support
- Configurable audio devices
- Built specifically for Fedora 43 with compatibility patches

## Limitations

- Video support disabled (due to FFmpeg 7.1 API changes)
- Database storage disabled (due to SOCI 4.x API changes)
- Encryption features disabled (ZRTP/LIME not included)

## Installation Instructions

### Simple One-Command Installation (Recommended)

**Just run this:**

```bash
sudo dnf install ./facetonesoftphone-1.0.0-1.fc43.x86_64.rpm
```

That's it! DNF will automatically:
- Download all required dependencies from Fedora repos (~30-50 MB)
- Install everything in the correct order
- Configure the application
- Make it available in your application menu

**No manual dependency installation needed!**

---

### Alternative: Manual Dependency Installation (Optional)

If you prefer to install dependencies separately first:


```bash
# Optional: Install dependencies manually first
sudo dnf install -y \
    qt6-qtbase \
    qt6-qtwebsockets \
    alsa-lib \
    pulseaudio-libs \
    speex \
    speexdsp \
    opus \
    sqlite-libs \
    libxml2 \
    zlib \
    openssl-libs \
    gsm \
    libjpeg-turbo \
    turbojpeg \
    libsrtp \
    mbedtls \
    xerces-c \
    jsoncpp

# Then install the RPM
sudo dnf install ./facetonesoftphone-1.0.0-1.fc43.x86_64.rpm
```

### Offline Installation (No Internet)


If you need to install on a system without internet:

1. On a connected Fedora 43 system, download all dependencies:
```bash
dnf download --resolve ./facetonesoftphone-1.0.0-1.fc43.x86_64.rpm
```

2. Copy all downloaded RPMs to the offline system

3. Install them all:
```bash
sudo rpm -ivh *.rpm
```

---

### Quick Start

Launch from terminal:
```bash
facetonesoftphone
```

Or search for "FacetoneSoftPhone" in your KDE application menu.

## Configuration

The configuration file is located at:
```
~/.config/Facetone/facetonesoftphone/config.json
```

Default configuration:
```json
{
    "WebSocketPort": 11000,
    "RemoteSipPort": "5060",
    "AutoAnswer": false,
    "APIPort": "11001",
    "useWss": false,
    "certPath": "cert.pem",
    "InputDevice": "",
    "OutputDevice": "",
    "RingtonePath": "ringtone.wav"
}
```

### Common Configuration Changes

**Change SIP Port:**
```bash
sed -i 's/"RemoteSipPort": "5060"/"RemoteSipPort": "5047"/' \
    ~/.config/Facetone/facetonesoftphone/config.json
```

**Enable Auto-Answer:**
```bash
sed -i 's/"AutoAnswer": false/"AutoAnswer": true/' \
    ~/.config/Facetone/facetonesoftphone/config.json
```

## Verification

After installation, verify the package:

```bash
# List installed files
rpm -ql facetonesoftphone | head -20

# Check package info
rpm -qi facetonesoftphone

# Verify the binary
which facetonesoftphone
facetonesoftphone --version
```

## Uninstallation

To remove the package:

```bash
sudo dnf remove facetonesoftphone
```

Or:

```bash
sudo rpm -e facetonesoftphone
```

## Troubleshooting

### Missing Dependencies

If you get dependency errors during installation:
```bash
# Install missing dependencies automatically
sudo dnf install ./facetonesoftphone-1.0.0-1.fc43.x86_64.rpm
```

### Port Already in Use

If you see "Address already in use" errors:
```bash
# Check what's using port 5060
sudo lsof -i :5060

# Change the port in config file (see Configuration section above)
```

### Audio Device Issues

List available audio devices:
```bash
aplay -L        # Playback devices
arecord -L      # Capture devices
```

Then update the config file with your preferred devices.

## Package Contents

The RPM installs files to:
- `/usr/local/bin/facetonesoftphone` - Main binary
- `/usr/local/lib64/` - Linphone libraries (static .a files)
- `/usr/local/include/` - Development headers
- `/usr/local/lib64/pkgconfig/` - pkg-config files
- `/usr/local/share/facetonesoftphone/` - Data files (ringtone)
- `/usr/local/share/applications/` - Desktop entry
- `/usr/local/share/icons/` - Application icon
- `/usr/local/share/doc/facetonesoftphone/` - Documentation

## System Requirements

- Fedora 43 (tested)
- x86_64 architecture
- ~80 MB disk space
- Audio device (microphone + speakers/headset)

## Compatibility Notes

This package was built specifically for **Fedora 43** with the following patches:

1. **mbedtls 3.6.5 compatibility**: Threading functions removed
2. **FFmpeg 7.1 compatibility**: Video support disabled
3. **SOCI 4.x compatibility**: Database storage disabled  
4. **GCC 15.2 compatibility**: Warning suppressions added

It may not work on other Fedora versions without rebuilding.

## Building from Source

If you need to build for a different Fedora version:

1. Clone the repository
2. Run the build script:
   ```bash
   ./build_fedora.sh
   ```
3. Create a new RPM:
   ```bash
   ./create_rpm_simple.sh
   ```

See `BUILDING_FEDORA.md` for detailed build instructions.

## Support

For issues or questions:
- Check `BUILDING_FEDORA.md` in the package documentation
- Review the build logs in `/tmp/`
- Ensure all dependencies are installed

## License

MIT License - See package documentation for details.

## Credits

Built with:
- Qt6 (UI framework)
- Linphone (SIP library stack)
- Multiple audio codecs (Speex, Opus, GSM)
- mbedtls (cryptography)

---

**Package Built On**: March 30, 2026
**For**: Fedora 43 KDE
**Version**: 1.0.0
