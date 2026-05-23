# Building Mangrove VST3 Plugin on Windows 11

**Target:** VST3 plugin with IGraphics UI for Windows 11 x64  
**Prerequisites:** Visual Studio 2022, Windows 10 SDK, Git with submodules  
**Estimated time:** 30–45 minutes (including first-time dependency download)

---

## Phase 0: Environment Validation

Before starting, verify your system has all required tools. Run these commands in PowerShell (Administrator):

```powershell
# Check Visual Studio installation
if (Get-Command devenv -ErrorAction SilentlyContinue) {
    Write-Host "✓ Visual Studio found"
    devenv /version
} else {
    Write-Host "✗ Visual Studio NOT found. Install Visual Studio 2022 with C++ workload"
    exit 1
}

# Check CMake (optional, but useful for troubleshooting)
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    Write-Host "✓ CMake found"
    cmake --version
} else {
    Write-Host "! CMake NOT found (optional, can use Visual Studio's built-in CMake)"
}

# Check Git
if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host "✓ Git found"
    git --version
} else {
    Write-Host "✗ Git NOT found. Install from git-scm.com"
    exit 1
}
```

**Expected output:**
```
✓ Visual Studio found
Microsoft Visual Studio 2022 Version 17.x.xxxxx
✓ CMake found
cmake version 3.x.x
✓ Git found
git version 2.x.x.windows.x
```

If any check fails, install the missing tool before proceeding.

---

## Phase 1: Clone Repository with Submodules

Navigate to where you want the project and clone with recursive submodule initialization:

```powershell
cd C:\Dev\  # or your preferred directory

git clone --recursive https://github.com/tweggen/nassau-mangrove2.git

cd nassau-mangrove2

# Verify submodules are initialized
git submodule status
```

**Expected output:**
```
 abc1234abcd1234abcd1234abcd1234abcd1234 external/iplug2 (HEAD detached at abc1234)
 def5678def5678def5678def5678def5678def5678 external/vst3sdk (HEAD detached at def5678)
```

If you see a minus sign (`-`) before either submodule path, they are not initialized. Run:

```powershell
git submodule update --init --recursive
```

---

## Phase 2: Open Visual Studio Project

Launch Visual Studio 2022 and open the Windows VST3 project:

```powershell
# Open directly from PowerShell
start ".\MangrovePlugin\projects\MangrovePlugin-vst3.vcxproj"

# Or open Visual Studio and File → Open → Project/Solution, then navigate to:
# MangrovePlugin\projects\MangrovePlugin-vst3.vcxproj
```

**In Visual Studio:**
- Solution Explorer appears on the right
- Right-click the project name `MangrovePlugin` → Properties

---

## Phase 3: Add CompressorChain DSP Library

### Step 3A: Add Source Files to Project

1. In Solution Explorer, right-click `MangrovePlugin` → **Add** → **Existing Item**
2. Navigate to: `Source\DSP\`
3. Select both files:
   - `compressor_chain.cpp`
   - `compressor_chain.h`
4. Click **Add**

Verify in Solution Explorer:
```
MangrovePlugin (project)
├── Source Files
│   ├── MangrovePlugin.cpp
│   ├── compressor_chain.cpp          ← newly added
│   └── (other source files)
├── Header Files
│   ├── MangrovePlugin.h
│   ├── compressor_chain.h            ← newly added
│   └── config.h
└── Resource Files
```

### Step 3B: Update Include Paths

1. Right-click `MangrovePlugin` → **Properties**
2. Ensure **Configuration:** `All Configurations` (top-left dropdown)
3. Navigate to: **C/C++** → **General** → **Additional Include Directories**
4. Add (one per line, or semicolon-separated on Windows):
   ```
   $(SolutionDir)\..\..\Source\DSP
   $(SolutionDir)\..\..\external\vst3sdk
   $(SolutionDir)\..\..\external\vst3sdk\pluginterfaces
   $(SolutionDir)\..\..\external\vst3sdk\public.sdk
   $(SolutionDir)\..\..\external\vst3sdk\base
   ```
5. Click **Apply** → **OK**

### Step 3C: Verify Library Linking

1. In **Properties**, go to **Linker** → **General**
2. Check **Additional Library Directories** — should already include:
   ```
   $(SolutionDir)\..\..\external\vst3sdk\build\lib\Release
   ```
   (or similar, depending on VST3 SDK build; IPlug2 template should handle this)

3. Go to **Linker** → **Input** → **Additional Dependencies**
4. Verify `sdk.lib` and `base.lib` are present (from VST3 SDK)

---

## Phase 4: Configure Build Settings

### Step 4A: Platform and Configuration

1. At the top of Visual Studio, find the dropdown menus:
   - **Solution Configuration:** Select `Release` (not Debug, for plugin performance)
   - **Solution Platform:** Select `x64` (64-bit Windows; `Win32` for 32-bit if needed)

### Step 4B: C++ Language Standard

1. **Properties** → **C/C++** → **Language**
2. **C++ Language Standard:** Set to `ISO C++17 (/std:c++17)` or later
3. Click **Apply** → **OK**

### Step 4C: Compiler Warnings (Optional)

To suppress IPlug2 third-party warnings:
1. **Properties** → **C/C++** → **General**
2. **Warning Level:** `W3` (medium) or `W4` (strict) — your choice
3. Apply

---

## Phase 5: Build VST3 Plugin

### Step 5A: Clean Build (First Time)

1. **Build** menu → **Clean Solution** (Ctrl+Alt+F7)
   - Wait for it to complete (progress bar at bottom)

### Step 5B: Build Project

1. **Build** menu → **Build Solution** (Ctrl+Shift+B)
   - Monitor the **Output** panel at the bottom for build progress
   - Expected build time: 1–3 minutes (longer on first build due to dependencies)

### Step 5C: Verify Build Success

**Expected Output (end of build log):**
```
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

**If build fails:**

| Error | Solution |
|-------|----------|
| `compressor_chain.h not found` | Verify Step 3B include paths; rebuild (Clean + Build) |
| `undefined reference to CompressorChain::init` | Ensure `compressor_chain.cpp` is in project (Step 3A) |
| `sdk.lib not found` | Build external VST3 SDK: `cmake --build build_vst3sdk --config Release` in repo root |
| `fatal error C1083: Cannot open include file: 'vst3sdk/...` | Update external/vst3sdk paths in Step 3B |
| `error LNK1104: cannot open file 'LIBCMT.lib'` | **Properties** → **C/C++** → **Code Generation** → **Runtime Library** → `/MD` (Multi-threaded DLL) |

---

## Phase 6: Locate Output Binary

After successful build, find the VST3 bundle:

```powershell
# In PowerShell, from repo root:
Get-ChildItem -Recurse -Filter "MangroveIPlug.vst3" -Type Directory

# Expected path (approximate):
# C:\Dev\nassau-mangrove2\MangrovePlugin\Build\Release\MangroveIPlug.vst3\
```

**Bundle structure should be:**
```
MangroveIPlug.vst3\
├── Contents\
│   ├── Resources\
│   │   └── (Info.plist, icon, fonts)
│   └── x86_64-win\
│       └── MangroveIPlug.vst3 (executable)
└── (VST3 standard bundle layout)
```

---

## Phase 7: Install Plugin

Copy the entire VST3 bundle to the Windows VST3 plugin directory:

```powershell
$vst3_dir = "C:\Program Files\Common Files\VST3"

# Verify directory exists
if (-Not (Test-Path $vst3_dir)) {
    Write-Host "VST3 directory does not exist. Creating..."
    New-Item -ItemType Directory -Force -Path $vst3_dir
}

# Copy plugin bundle
$source = ".\MangrovePlugin\Build\Release\MangroveIPlug.vst3"
$dest = "$vst3_dir\MangroveIPlug.vst3"

if (Test-Path $source) {
    Copy-Item -Recurse -Force $source $dest
    Write-Host "✓ Plugin installed to: $dest"
} else {
    Write-Host "✗ Source not found: $source"
    Write-Host "   Check Phase 5 build output and Phase 6 path"
}
```

---

## Phase 8: Validate Plugin in DAW

### Step 8A: Rescan VST3 Plugins

Open your DAW (e.g., **Reaper**, **Ableton Live**, **Studio One**, **Cubase**):

**Reaper:**
```
Options → Preferences → Audio → VST
  → Rescan VST plug-ins (click button)
  → Wait for scan to complete
```

**Ableton Live:**
```
Preferences → File/Folder → Add Library Folder
  → Select: C:\Program Files\Common Files\VST3\
  → Restart Ableton
```

**Studio One:**
```
Studio One → Options → Locations → Plug-in folders
  → Verify VST3 folder is listed
  → Right-click in browser → Scan for Plug-ins
```

### Step 8B: Verify Plugin Appears

1. In the DAW's plugin browser, search for **"Mangrove"** or **"Nassau"**
2. Plugin should appear as: **Nassau: Mangrove** (version 5.0.0)
3. Double-click to load; UI window should open (640×400 canvas with knobs)

### Step 8C: Test Audio and Parameters

1. **Audio path:**
   - Route audio into the track with Mangrove plugin
   - Play audio file or generate tone
   - Verify sound passes through (plug may be transparent if all parameters are at defaults)

2. **Parameter interaction:**
   - Click knobs in the UI; verify they move smoothly
   - Check DAW's parameter list (automation track); should show all 14 parameters:
     - Input: Gain, Lo Cut, Saturate
     - Level: Threshold, Ratio, Attack, Release, Lo Cut, Tube Gain, Feedback
     - Density: Threshold, Ratio, Attack, Release

3. **DSP verification:**
   - Adjust **Input Gain** knob; watch output level change
   - Enable **Level Feedback**; observe compressor behavior
   - Sweep **Density Threshold**; audition compression stages

### Step 8D: Success Criteria

✅ **Plugin loads without crash**  
✅ **UI displays all 14 controls**  
✅ **Parameters are automatable**  
✅ **Audio passes through in real-time**  
✅ **No MSVC runtime errors in DAW console**

---

## Phase 9: Troubleshooting

### Plugin loads but crashes immediately

**Cause:** DSP initialization or ProcessBlock error  
**Fix:**
1. Check **Output** panel in Visual Studio for build warnings
2. Rebuild with `/MD` (multi-threaded DLL) runtime library
3. If IPlug2 wrapper issue, check `Source/Plugin/MangrovePlugin.cpp` `OnReset()` method

### UI doesn't appear (plugin loads silently)

**Cause:** IGraphics/OpenGL initialization  
**Fix:**
1. Verify OpenGL headers in `external/iplug2/Dependencies/IGraphics/glad/include/`
2. Check `MangrovePlugin/config/MangrovePlugin-win.props` for `IGRAPHICS_GL=1`
3. Update GPU drivers (Intel/NVIDIA/AMD)

### "Missing DLL" error on plugin load

**Cause:** Runtime dependencies not found  
**Fix:**
1. Run Visual Studio **Build** → **Build Solution** again (full rebuild)
2. Check project **Linker** → **General** → **Use Library Dependency Inputs:** `Yes`
3. Verify `external/vst3sdk/build/lib/Release/*.lib` exist; if not:
   ```powershell
   cd external/vst3sdk
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   cd ../..
   ```

### Parameters don't show in automation list

**Cause:** Parameter initialization mismatch  
**Fix:**
1. Check `Source/Plugin/config.h`: `#define PLUG_N_PARAMS 14`
2. Check `Source/Plugin/MangrovePlugin.cpp`: Constructor initializes exactly 14 params
3. Rebuild and rescan in DAW

### Audio clips or distorts

**Cause:** Sample type conversion or buffer overflow  
**Fix:**
1. Check `MangrovePlugin.cpp` `ProcessBlock()`: Sample type is `iplug::sample` (double), converted to float for DSP
2. Verify buffer sizes: `kMaxBlockSize = 8192` is sufficient
3. Lower **Input Gain** in plugin UI and test again

---

## Phase 10: Next Steps (Optional)

Once VST3 builds and validates:

1. **Build AUv3 (macOS only):** Not applicable on Windows; skip
2. **Build AAX (if you have AAX SDK):** Use `MangrovePlugin/projects/MangrovePlugin-aax.vcxproj`
3. **Build CLAP:** Use `MangrovePlugin/projects/MangrovePlugin-clap.vcxproj`
4. **Create Installer:** Windows installer would distribute `MangroveIPlug.vst3` to standard location
5. **Code Sign:** Add Windows code-signing certificate to prevent UAC warnings

---

## Testing Checklist

Print this and tick off each step:

- [ ] **Phase 0:** Environment validated (VS, Git, CMake optional)
- [ ] **Phase 1:** Repository cloned with submodules initialized
- [ ] **Phase 2:** Visual Studio project opened
- [ ] **Phase 3A:** `compressor_chain.cpp/.h` added to project
- [ ] **Phase 3B:** Include paths updated (Source/DSP, external/vst3sdk)
- [ ] **Phase 4:** Build configuration set to Release x64, C++17
- [ ] **Phase 5:** Build succeeded (0 failed)
- [ ] **Phase 6:** Output binary found at expected path
- [ ] **Phase 7:** Plugin copied to `C:\Program Files\Common Files\VST3\`
- [ ] **Phase 8A:** DAW rescan completed without errors
- [ ] **Phase 8B:** Plugin appears in DAW browser
- [ ] **Phase 8C:** Audio passes through, parameters respond to input
- [ ] **Phase 8D:** All success criteria met

---

## Reference: Key File Locations

```
Repository Root (C:\Dev\nassau-mangrove2\)
├── Source/
│   ├── DSP/
│   │   ├── compressor_chain.cpp      ← Add to project (Phase 3A)
│   │   └── compressor_chain.h
│   ├── Plugin/                       ← IPlug2 wrapper (reference)
│   │   ├── MangrovePlugin.cpp
│   │   ├── config.h
│   │   └── MangroveUI.cpp
│   └── VST3/                         ← Phase 4 native VST3 (fallback)
│
├── MangrovePlugin/
│   ├── projects/
│   │   ├── MangrovePlugin-vst3.vcxproj    ← OPEN THIS FILE (Phase 2)
│   │   ├── MangrovePlugin-clap.vcxproj
│   │   └── MangrovePlugin-aax.vcxproj
│   ├── Build/
│   │   └── Release/
│   │       └── MangroveIPlug.vst3/        ← Output binary (Phase 6)
│   ├── config/
│   │   └── MangrovePlugin-win.props       ← Windows config
│   ├── MangrovePlugin.cpp
│   ├── config.h
│   └── README.md
│
├── external/
│   ├── vst3sdk/                      ← Steinberg VST3 SDK (submodule)
│   └── iplug2/                       ← IPlug2 framework (submodule)
│
└── docs/
    ├── BUILDING.md                   ← General build guide
    └── BUILDING_WIN11.md             ← THIS FILE
```

---

## Support & Troubleshooting Resources

- **IPlug2 Documentation:** https://github.com/iPlug2/iPlug2/wiki
- **VST3 SDK:** https://github.com/steinbergmedia/vst3sdk
- **Windows Audio Dev:** https://docs.microsoft.com/en-us/windows/win32/audio/core-audio-interfaces
- **This repo issues:** Report build errors at https://github.com/tweggen/nassau-mangrove2/issues

---

**Last Updated:** 2026-05-23  
**Tested on:** Windows 11 x64, Visual Studio 2022 (v17.x)  
**Contact:** Timo Weggen (timo.weggen@gmail.com)
