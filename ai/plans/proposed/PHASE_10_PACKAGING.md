# Phase 10: Packaging & Release

**Objective:** Create distribution packages, documentation, and release infrastructure.

**Duration:** 2–3 weeks  
**Dependencies:** Phase 1–9 (All complete)  
**Produces:** Installers (Windows, macOS), packages (Linux), documentation  

---

## Overview

Final phase prepares the plugin for public release:

- ✅ Installers (Windows: .exe or .msi, macOS: .dmg)
- ✅ Code signing & notarization (macOS)
- ✅ Linux packages (deb, rpm, Flatpak)
- ✅ User documentation (manual, parameter guide)
- ✅ Factory presets
- ✅ Version management

---

## Detailed Tasks

### Task 10.1: Windows Installer (NSIS)

**Duration:** 3–4 days

**File:** `installer/Mangrove.nsi`

```nsis
; NSIS Installer for Mangrove VST3

!include "MUI2.nsh"

; Product Info
!define PRODUCT_NAME "Mangrove Compressor"
!define PRODUCT_VERSION "2.0.0"
!define PRODUCT_PUBLISHER "Nassau"
!define PRODUCT_WEB_SITE "https://example.com"

; Directories
!define VST3_DIR "$PROGRAMFILES\Common Files\VST3"

SetCompress force
SetCompressor /SOLID lzma

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Mangrove-${PRODUCT_VERSION}-Windows-x64.exe"
InstallDir "${VST3_DIR}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "Install"
  SetOutPath "${VST3_DIR}\Mangrove.vst3\Contents\x86_64-win"
  File "Mangrove.vst3\Contents\x86_64-win\Mangrove.vst3"
  
  SetOutPath "$SMPROGRAMS\Mangrove"
  CreateShortCut "$SMPROGRAMS\Mangrove\Uninstall.lnk" \
    "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
  RMDir /r "${VST3_DIR}\Mangrove.vst3"
  RMDir /r "$SMPROGRAMS\Mangrove"
SectionEnd
```

**Build installer:**
```bash
# Windows
makensis installer/Mangrove.nsi
# Output: Mangrove-2.0.0-Windows-x64.exe
```

---

### Task 10.2: macOS Disk Image & Code Signing

**Duration:** 4–5 days

**Code Signing (Required for distribution):**

```bash
# Sign VST3 plugin
codesign --deep --force --verify --verbose \
  --sign "Developer ID Application: Nassau (XXXXXXXXXX)" \
  build/Mangrove.vst3

# Sign AU plugin
codesign --deep --force --verify --verbose \
  --sign "Developer ID Application: Nassau (XXXXXXXXXX)" \
  build/Mangrove.auv3

# Verify signatures
codesign --verify --deep --strict build/Mangrove.vst3
codesign --verify --deep --strict build/Mangrove.auv3

# Notarize (for App Store / Gatekeeper acceptance)
xcrun altool --notarize-app \
  --file Mangrove-2.0.0-macOS-x64.dmg \
  --primary-bundle-id com.nassau.mangrove.vst3 \
  --username user@example.com \
  --password @keychain:altool-password
```

**Create DMG installer:**

```bash
# Create DMG
hdiutil create -volname "Mangrove 2.0.0" \
  -srcfolder build/install_mac \
  -ov -format UDZO \
  Mangrove-2.0.0-macOS-x64.dmg

# Sign DMG
codesign --sign "Developer ID Application: Nassau" \
  Mangrove-2.0.0-macOS-x64.dmg

# Notarize DMG
xcrun altool --notarize-app \
  --file Mangrove-2.0.0-macOS-x64.dmg \
  --primary-bundle-id com.nassau.mangrove \
  --username user@example.com
```

---

### Task 10.3: Linux Packages

**Duration:** 2–3 days

**Debian/Ubuntu (.deb):**

```bash
# Create package structure
mkdir -p mangrove_2.0.0/DEBIAN
mkdir -p mangrove_2.0.0/usr/lib/vst3
mkdir -p mangrove_2.0.0/usr/lib/ladspa

# Copy files
cp -r build/Mangrove.vst3 mangrove_2.0.0/usr/lib/vst3/
cp build/libmangrove_ladspa.so mangrove_2.0.0/usr/lib/ladspa/

# Create control file
cat > mangrove_2.0.0/DEBIAN/control << EOF
Package: mangrove-compressor
Version: 2.0.0
Architecture: amd64
Maintainer: Nassau <info@nassau.de>
Homepage: https://example.com
Description: Mangrove Compressor - Dual-stage dynamic range compressor
 VST3 and LADSPA plugin for audio compression.
EOF

# Build package
dpkg-deb --build mangrove_2.0.0
# Output: mangrove-compressor_2.0.0_amd64.deb
```

**Flatpak (universal Linux):**

```yaml
# flatpak/com.nassau.mangrove.yaml
id: com.nassau.mangrove
runtime: org.freedesktop.Platform
sdk: org.freedesktop.Sdk

build-options:
  cflags: -O2 -pipe
  cxxflags: -O2 -pipe

modules:
  - name: mangrove
    buildsystem: cmake
    sources:
      - type: git
        url: https://github.com/nassau/mangrove.git
        branch: main
    build-options:
      env:
        IPLUG_PLUGINFORMATS: VST3;LADSPA
    post-install:
      - install -d /app/lib/vst3
      - install -d /app/lib/ladspa
      - cp -r build/Mangrove.vst3 /app/lib/vst3/
      - cp build/libmangrove_ladspa.so /app/lib/ladspa/
```

**Build Flatpak:**
```bash
flatpak-builder build-dir --force-clean flatpak/com.nassau.mangrove.yaml
flatpak build-export repo build-dir
flatpak build-bundle repo mangrove-2.0.0.flatpak com.nassau.mangrove
```

---

### Task 10.4: Version Management

**Duration:** 1 day

**Create version.h:**

```cpp
#pragma once

#define MANGROVE_VERSION_MAJOR 2
#define MANGROVE_VERSION_MINOR 0
#define MANGROVE_VERSION_PATCH 0

#define MANGROVE_VERSION_STRING "2.0.0"
#define MANGROVE_VERSION_BUILD_DATE "2026-05-18"

// Plugin format versions
#define MANGROVE_VST3_VERSION "2.0.0"
#define MANGROVE_AU_VERSION "2.0.0"
#define MANGROVE_LADSPA_VERSION "2.0.0"
```

**Git tagging:**

```bash
# Create version tag
git tag -a v2.0.0 -m "Release version 2.0.0"

# Update CHANGELOG.md
echo "## v2.0.0 (2026-05-18)
- Initial release
- VST3, AudioUnit v3, LADSPA support
- Dual-stage compression (Level + Density)
- 14 parameters
- Cross-platform (Windows, macOS, Linux)" >> CHANGELOG.md

# Commit and push
git add CHANGELOG.md
git commit -m "Release v2.0.0"
git push origin main --tags
```

---

### Task 10.5: Factory Presets

**Duration:** 2–3 days

Create 10–15 factory presets:

```json
// presets/Default.json
{
  "version": 1,
  "name": "Default",
  "parameters": {
    "inputGain": 0.0,
    "inputLoCut": 80.0,
    "inputSaturate": 0.0,
    "levelThreshold": -10.0,
    "levelRatio": 2.5,
    "levelAttack": 10.0,
    "levelRelease": 300.0,
    "levelLoCut": false,
    "levelTubeGain": false,
    "levelFeedback": true,
    "densityThreshold": -10.0,
    "densityRatio": 1.0,
    "densityAttack": 10.0,
    "densityRelease": 300.0
  }
}

// presets/Vocals.json
{
  "name": "Vocal Compression",
  "parameters": {
    "levelThreshold": -12.0,
    "levelRatio": 4.0,
    "levelAttack": 5.0,
    "levelRelease": 150.0,
    "densityThreshold": -8.0,
    "densityRatio": 2.0,
    ...
  }
}

// presets/Drums.json
{
  "name": "Drum Glue",
  "parameters": {
    "levelThreshold": -15.0,
    "levelRatio": 3.0,
    "levelAttack": 2.0,
    "levelRelease": 100.0,
    "levelFeedback": false,
    ...
  }
}

// ... more presets (Bass, Bass + Parallel, Pumping, etc.)
```

**Install presets:**
```bash
mkdir -p ~/.config/mangrove/presets
cp presets/*.json ~/.config/mangrove/presets/
```

---

### Task 10.6: User Documentation

**Duration:** 3–4 days

Create user manual:

```markdown
# Mangrove Compressor - User Guide

## Installation

### Windows
1. Download Mangrove-2.0.0-Windows-x64.exe
2. Run installer
3. Plugin appears in VST3 folder

### macOS
1. Download Mangrove-2.0.0-macOS-x64.dmg
2. Open .dmg
3. Drag Mangrove.vst3 to /Library/Audio/Plug-Ins/VST3/
4. (Optional) Drag Mangrove.auv3 to /Library/Audio/Plug-Ins/Components/

### Linux
```bash
sudo apt install ./mangrove-compressor_2.0.0_amd64.deb
# or
flatpak install mangrove-2.0.0.flatpak
```

## Parameters

### Input Stage
- **Input Gain** (-24 to +24 dB): Adjust input level
- **Input LoC ut** (20–300 Hz): High-pass filter cutoff
- **Input Saturate** (0–5): Soft-clipping nonlinearity

### Level Compressor
- **Threshold** (-60 to 0 dB): Compression onset
- **Ratio** (1–10, ≥9.99 = Vari-Mu): Compression slope
- **Attack** (0–100 ms): Time to react to peaks
- **Release** (10–500 ms): Time to recover
- **LoC ut**: Enable sidechain high-pass
- **TubeGain**: Soft-clipping saturation
- **Feedback**: Feedback vs. feedforward mode

### Density Compressor
- **Threshold** (-36 to 0 dB): Fast limiter onset
- **Ratio** (1–10, ≥9.99 = Limiter): Limiting slope
- **Attack** (0.001–100 ms): Transient catching
- **Release** (10–2000 ms): Gentle recovery

## Quick Start

1. **Vocal Compression:** Load "Vocal Compression" preset
2. **Drum Glue:** Load "Drum Glue" preset
3. **Bass:** Load "Bass" preset

## Tips & Tricks

- Use **Level LoC ut** for digital artifacts removal
- Enable **TubeGain** for warmth on harsh sources
- Switch **Feedback** to **Feedforward** for transparent gain reduction
- Use **Density** for transient control without coloring tone

## Technical Specs

- Latency: 0 samples
- CPU: <1% per instance
- Memory: 2 MB per instance
- Supported sample rates: 44.1 to 192 kHz
- I/O: Stereo in/out

## Support

Visit: https://example.com/support
```

---

### Task 10.7: Release Checklist

**Duration:** 2–3 days

Final verification before release:

```markdown
## Release Checklist v2.0.0

### Code & Build
- [ ] All tests pass (Phase 9)
- [ ] No compiler warnings
- [ ] Git history clean
- [ ] Version number updated (v2.0.0)
- [ ] CHANGELOG.md up-to-date

### Installers
- [ ] Windows installer builds
- [ ] macOS code signing complete
- [ ] macOS notarization complete
- [ ] Linux .deb package builds
- [ ] Linux Flatpak builds
- [ ] All installers tested

### Documentation
- [ ] User manual complete
- [ ] Parameter guide complete
- [ ] Presets documented
- [ ] Installation instructions clear
- [ ] Support contact listed

### Marketing
- [ ] Website updated
- [ ] Release notes written
- [ ] Screenshots prepared
- [ ] Demo video ready (optional)

### Distribution
- [ ] GitHub release created (with assets)
- [ ] Installers uploaded to website
- [ ] Announce on social media
- [ ] Email list notification

### Post-Release
- [ ] Monitor for crash reports
- [ ] Support email monitored
- [ ] v2.0.1 patch plan ready (if needed)
```

---

## Deliverables

- [ ] `Mangrove-2.0.0-Windows-x64.exe` (installer)
- [ ] `Mangrove-2.0.0-macOS-x64.dmg` (disk image)
- [ ] `mangrove-compressor_2.0.0_amd64.deb` (Debian)
- [ ] `com.nassau.mangrove.flatpak` (Flatpak)
- [ ] `Mangrove_User_Guide.pdf` (documentation)
- [ ] `CHANGELOG.md` (release notes)
- [ ] `presets/*.json` (factory presets)
- [ ] GitHub Release (with download links)

---

## Success Criteria

✅ Installers created for all platforms  
✅ Code signing & notarization complete  
✅ User documentation complete  
✅ Factory presets included  
✅ Version tagging in Git  
✅ Installation tested on clean systems  
✅ Release notes published  
✅ Website/distribution channels updated  

---

## Timeline

Days 1–2: Tasks 10.1 (Windows)  
Days 3–4: Task 10.2 (macOS signing)  
Days 5–6: Task 10.3 (Linux packages)  
Days 7: Task 10.4–10.5 (versioning + presets)  
Days 8–9: Task 10.6 (documentation)  
Days 10: Task 10.7 (final checklist + release)  

**Total: 2–3 weeks**

---

## Post-Release

### Monitoring
- Watch support email for crash reports
- Monitor GitHub issues
- Collect user feedback

### Maintenance
- Plan v2.0.1 if critical bugs found
- Plan v2.1.0 for new features (optional plugins: VST 2, Standalone)

### Success Metrics
- Download count
- User feedback/reviews
- Community activity

---

## Resources

- **NSIS:** https://nsis.sourceforge.io/
- **macOS Code Signing:** https://developer.apple.com/support/code-signing/
- **Debian Packaging:** https://www.debian.org/doc/
- **Flatpak:** https://docs.flatpak.org/
