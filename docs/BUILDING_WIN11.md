# Building Mangrove VST3 Plugin on Windows 11

**Target:** VST3 plugin with IGraphics UI for Windows 11 x64  
**Prerequisites:** Visual Studio 2022 or newer, Windows 10/11 SDK, Git with submodules  
**Estimated time:** 30–45 minutes (including first-time dependency download)

> **In a hurry?** The whole build is one command from the repository root:
>
> ```powershell
> msbuild MangrovePlugin\MangrovePlugin.sln /t:"MangrovePlugin-vst3" `
>   /p:Configuration=Release /p:Platform=x64 /m
> ```
>
> This produces the plugin **with** the custom UI and installs it to
> `%LOCALAPPDATA%\Programs\Common\VST3\`. No Skia setup is required on Windows — IGraphics
> uses the NanoVG/OpenGL 2 backend here. Close your DAW first, or the install step fails
> with `Sharing violation`.

> ⚠️ **Do not use `BUILD.md` on Windows.** That guide drives the root CMake project, which is
> macOS-only: it passes GCC/Clang flags to MSVC, omits `NOMINMAX`, and its graphics sources
> are Objective-C++. It cannot build the Windows plugin, and where it does configure it
> disables the UI entirely. This file is the Windows path.

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

## Phase 2: Open the Solution

> ⚠️ **Open the `.sln`, never the `.vcxproj`.** The project files import their property
> sheets via `$(SolutionDir)` (`MangrovePlugin-vst3.vcxproj` lines 82–102), which only
> resolves when the build goes through the solution. Opening or building the `.vcxproj`
> directly fails with:
>
> ```
> error MSB4019: The imported project "...\projects\config\MangrovePlugin-win.props"
> was not found.
> ```

```powershell
# Open directly from PowerShell
start ".\MangrovePlugin\MangrovePlugin.sln"

# Or open Visual Studio and File → Open → Project/Solution, then navigate to:
# MangrovePlugin\MangrovePlugin.sln
```

The solution contains five targets; the one you want is **MangrovePlugin-vst3**
(`-app`, `-vst2`, `-aax` and `-clap` are the other formats).

**In Visual Studio:**
- Solution Explorer appears on the right
- Right-click **MangrovePlugin-vst3** → **Set as Startup Project**

---

## Phase 3: Verify Project Wiring (no changes needed)

> **Nothing to do here — this phase is a sanity check only.** Earlier revisions of this
> guide asked you to add the DSP sources and include paths by hand. They are already
> committed to the project files, and re-adding them creates duplicate entries. Only read
> on if the build fails.

The DSP library, the UI, and their include paths ship in the checked-in project files:

| What | Where it is declared |
|---|---|
| `compressor_chain.cpp` / `.h` | `MangrovePlugin-vst3.vcxproj` lines 394 / 330 |
| `MangroveUI.cpp` / `.h` | `MangrovePlugin-vst3.vcxproj` lines 395 / 331 |
| `Source\DSP` + `Source\Plugin` include paths | `MangrovePlugin\config\MangrovePlugin-win.props` line 22 |
| Graphics backend (`IGRAPHICS_NANOVG;IGRAPHICS_GL2`) | `MangrovePlugin\config\MangrovePlugin-win.props` line 6 |

Note that the VST3 SDK used on Windows is the copy vendored inside IPlug2
(`external\iplug2\Dependencies\IPlug\VST3_SDK`), compiled from source as part of the
project — **not** `external\vst3sdk`, and there is no `sdk.lib`/`base.lib` to build or link.

---

## Phase 4: Select Configuration

The compiler settings (C++17, warning level, runtime library) all come from
`MangrovePlugin-win.props`. The only thing you choose is the configuration:

- **Solution Configuration:** `Release` (not Debug, for plugin performance)
- **Solution Platform:** `x64`

---

## Phase 5: Build VST3 Plugin

### Step 5A: Command line (recommended)

From the repository root:

```powershell
msbuild MangrovePlugin\MangrovePlugin.sln /t:"MangrovePlugin-vst3" `
  /p:Configuration=Release /p:Platform=x64 /m
```

If `msbuild` is not on your PATH, either run this from a **Developer PowerShell for VS**, or
locate it with `vswhere`:

```powershell
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
        -latest -requires Microsoft.Component.MSBuild -property installationPath
$msbuild = Join-Path $vs "MSBuild\Current\Bin\MSBuild.exe"
& $msbuild MangrovePlugin\MangrovePlugin.sln /t:"MangrovePlugin-vst3" `
  /p:Configuration=Release /p:Platform=x64 /m
```

### Step 5B: Or from Visual Studio

1. **Build** menu → **Build Solution** (Ctrl+Shift+B)
   - Monitor the **Output** panel for progress
   - Expected build time: 1–3 minutes (longer on the first build)

### Step 5C: Verify Build Success

A successful run ends with the postbuild step reporting the bundle copy:

```
MangrovePlugin-vst3.vcxproj -> ...\build-win\vst3\x64\Release\MangrovePlugin.vst3
copying 64bit binary to VST3 BUNDLE ...
copying VST3 bundle to 64bit VST3 Plugins folder ...
        1 File(s) copied
```

Two messages during a *successful* build are harmless and expected:

- `icudtl.dat not found ..., skipping` — ICU data is only needed by the Skia backend;
  Windows uses NanoVG/GL2.
- Four `IPlug_include_in_plug_hdr.h` warnings about `SHARED_RESOURCES_SUBPATH`,
  `PLUG_URL_STR`, `PLUG_EMAIL_STR`, `PLUG_COPYRIGHT_STR` being undefined.

**If the build fails:** see Phase 9.

---

## Phase 6: Locate Output Binary

The build produces a loose DLL and, via the postbuild script, a VST3 bundle:

```powershell
# In PowerShell, from repo root:
Get-ChildItem -Recurse -Filter "MangrovePlugin.vst3" MangrovePlugin\build-win
```

```
MangrovePlugin\build-win\
├── vst3\x64\Release\MangrovePlugin.vst3      ← linker output (plain DLL)
└── MangrovePlugin.vst3\                      ← assembled bundle
    └── Contents\
        ├── x86_64-win\
        │   └── MangrovePlugin.vst3           (the DLL, ~1.0 MB)
        └── Resources\
```

> The binary is named **`MangrovePlugin.vst3`** and lives under **`build-win\`**. Older
> revisions of this guide referred to `MangroveIPlug.vst3` under `Build\Release\` — that
> path has never existed in this repo. `MangroveIPlug` is the name used by the separate
> root-CMake macOS target (see `BUILD.md`), not by the Windows build.

---

## Phase 7: Install Plugin

**Normally there is nothing to do — the build installs the plugin for you.**
`scripts\postbuild-win.bat` copies the finished bundle to the per-user VST3 folder:

```
%LOCALAPPDATA%\Programs\Common\VST3\MangrovePlugin.vst3\
```

Confirm it landed:

```powershell
Get-ChildItem -Recurse "$env:LOCALAPPDATA\Programs\Common\VST3\MangrovePlugin.vst3" -File |
  Select-Object FullName, Length, LastWriteTime
```

To install machine-wide instead (all users, needs an elevated shell):

```powershell
$dest = "C:\Program Files\Common Files\VST3"
New-Item -ItemType Directory -Force -Path $dest | Out-Null
Copy-Item -Recurse -Force ".\MangrovePlugin\build-win\MangrovePlugin.vst3" $dest
```

> **Close your DAW before rebuilding.** If a host still has the installed plugin loaded, the
> postbuild copy fails with `Sharing violation` and `error MSB3073 ... exited with code 4`,
> *after* the compile and link have already succeeded. See Phase 9.

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
Preferences → Plug-Ins → VST3 Plug-In Custom Folder
  → Select: %LOCALAPPDATA%\Programs\Common\VST3\
  → Rescan
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
   - Check DAW's parameter list (automation track); should show all 15 parameters:
     - Input (3): Gain, Lo Cut, Saturate
     - Level (8): Threshold, Ratio, Attack, Release, Lo Cut, Tube Gain, Feedback, Fast
     - Density (4): Threshold, Ratio, Attack, Release

3. **DSP verification:**
   - Adjust **Input Gain** knob; watch output level change
   - Enable **Level Feedback**; observe compressor behavior
   - Sweep **Density Threshold**; audition compression stages

### Step 8D: Success Criteria

✅ **Plugin loads without crash**  
✅ **UI displays all 15 controls**  
✅ **Parameters are automatable**  
✅ **Audio passes through in real-time**  
✅ **No MSVC runtime errors in DAW console**

---

## Phase 9: Troubleshooting

### Plugin loads but crashes immediately

**Cause:** DSP initialization or ProcessBlock error  
**Fix:**
1. Check **Output** panel in Visual Studio for build warnings
2. If IPlug2 wrapper issue, check `MangrovePlugin/MangrovePlugin.cpp` `OnReset()` method

### UI doesn't appear (plugin loads silently)

**Cause:** IGraphics/OpenGL initialization  
**Fix:**
1. Confirm the UI was actually compiled in — the built DLL should contain the string
   `IGraphicsWin` and the section labels `INPUT` / `LEVEL` / `DENSITY`:
   ```powershell
   $dll = "MangrovePlugin\build-win\MangrovePlugin.vst3\Contents\x86_64-win\MangrovePlugin.vst3"
   $txt = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($dll))
   "IGraphicsWin","INPUT","LEVEL","DENSITY" | % { "$_ = $($txt.Contains($_))" }
   ```
   If these are missing you built a DSP-only variant — check you used the solution (Phase 5)
   and not the root CMake project.
2. Check `MangrovePlugin/config/MangrovePlugin-win.props` line 6 for
   `IGRAPHICS_NANOVG;IGRAPHICS_GL2`, and `MangrovePlugin/config.h` for `PLUG_HAS_UI 1`
3. Update GPU drivers (Intel/NVIDIA/AMD)

### `error MSB4019: The imported project ...\projects\config\MangrovePlugin-win.props was not found`

**Cause:** You built the `.vcxproj` directly. Its property-sheet imports use
`$(SolutionDir)`, which then wrongly resolves to `projects\`.
**Fix:** Build through `MangrovePlugin\MangrovePlugin.sln` — see Phase 2.

### `Sharing violation` / `error MSB3073 ... exited with code 4`

**Cause:** Not a compile error. The compile and link succeeded; the *postbuild install*
could not overwrite the installed bundle because a DAW (or an open Explorer window) still
holds `MangrovePlugin.vst3` open.
**Fix:** Close the DAW, then rebuild. The loose DLL from the failed run is still valid at
`MangrovePlugin\build-win\vst3\x64\Release\MangrovePlugin.vst3`.

### `error C1083: Cannot open include file: "MangroveUI.h"`

**Cause:** A stale `MangrovePlugin-win.props` missing the `Source\Plugin` include path.
**Fix:** Confirm line 22 of `MangrovePlugin\config\MangrovePlugin-win.props` ends with
`$(ProjectDir)..\..\Source\Plugin`. This was fixed in-tree; you should not hit it.

### `error D8021: invalid numeric argument '/Wextra'` or `error C2589: '(' : illegal token on right side of '::'`

**Cause:** You are building the **root CMake project** (`BUILD.md`), which is macOS-only.
It passes GCC/Clang flags to MSVC and omits `NOMINMAX`, and its graphics sources are
Objective-C++ (`IGraphicsMac.mm`, `macmain.cpp`).
**Fix:** Use the solution instead (Phase 5). The root CMake project cannot build the
Windows plugin, and even where it configures it disables the UI entirely.

### "Missing DLL" error on plugin load

**Cause:** Runtime dependencies not found  
**Fix:**
1. Rebuild the solution from scratch (**Build** → **Clean Solution**, then **Build Solution**)
2. Confirm the bundle contains `Contents\x86_64-win\MangrovePlugin.vst3` (~1.0 MB)
3. The VST3 SDK is compiled from IPlug2's vendored copy as part of the project; there are
   no separate `.lib` files to build under `external\vst3sdk`

### Parameters don't show in automation list

**Cause:** Parameter initialization mismatch  
**Fix:**
1. Check `MangrovePlugin/config.h`: `#define PLUG_N_PARAMS 15`
2. Check `MangrovePlugin/MangrovePlugin.cpp`: constructor initializes exactly 15 params,
   matching `kNumParams` in `MangrovePlugin/MangrovePlugin.h`
3. Rebuild and rescan in DAW

> **Watch out for the duplicate-config trap.** `Source/Plugin/MangrovePlugin.h` and
> `Source/Plugin/config.h` are near-copies of the pair in `MangrovePlugin/`. Because MSVC
> resolves quoted includes relative to the including file first, `Source/Plugin/MangroveUI.cpp`
> compiles against the **`Source/Plugin`** copies — so editing only `MangrovePlugin/config.h`
> may not reach it. Edit both, or consolidate them.

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
2. **Build AAX (if you have AAX SDK):** Build the `MangrovePlugin-aax` target from the solution
3. **Build CLAP:** See "Building CLAP" below — this target works today
4. **Create Installer:** Windows installer would distribute `MangrovePlugin.vst3` to standard location
5. **Code Sign:** Add Windows code-signing certificate to prevent UAC warnings

---

## Building CLAP

CLAP is built alongside VST3 from the same solution and the same plugin sources. It is a
plain `.clap` DLL — unlike VST3 there is no bundle folder to create.

### One-time prerequisite: download the CLAP SDKs

The iPlug2 submodule ships the CLAP *wrapper* but not the CLAP *SDK headers* —
`external/iplug2/Dependencies/IPlug/CLAP_SDK/` and `CLAP_HELPERS/` contain only a
`readme.txt` on a fresh clone. Fetch them with iPlug2's own script (needs Git Bash):

```bash
cd external/iplug2/Dependencies/IPlug
./download-clap-sdks.sh
```

Both repos must come from the **same** revision line. `clap-helpers` publishes no release
tags and its `main` tracks draft extensions in clap's `main` (e.g. `clap_host_param_hovered`),
so pinning the SDK to a release tag — `./download-clap-sdks.sh 1.2.9` — fails to compile
with errors in `host-proxy.hh`. Take the default (`main` for both), which is the combination
iPlug2's own CI builds against.

Both directories are covered by iPlug2's `.gitignore`, so this does not dirty the submodule.
Verify afterwards:

```
external/iplug2/Dependencies/IPlug/CLAP_SDK/include/clap/clap.h
external/iplug2/Dependencies/IPlug/CLAP_HELPERS/include/clap/helpers/plugin.hh
```

### One-time prerequisite: create the CLAP install folder

`scripts/postbuild-win.bat` installs the built plugin only *if* the target folder already
exists, and Windows has no CLAP folder by default. Create it once, or the build will
silently succeed without installing:

```powershell
New-Item -ItemType Directory -Force "$env:LOCALAPPDATA\Programs\Common\CLAP"
```

### Build

Build **through the solution**, not the `.vcxproj` directly — the property sheets resolve
via `$(SolutionDir)`, so a direct project build fails with `MSB4019: The imported project
...\projects\config\MangrovePlugin-win.props was not found`.

```powershell
msbuild MangrovePlugin\MangrovePlugin.sln /t:"MangrovePlugin-clap" `
  /p:Configuration=Release /p:Platform=x64 /m
```

Or in Visual Studio: open `MangrovePlugin.sln`, set **MangrovePlugin-clap** as the startup
project, Release | x64, Build.

Output:

```
MangrovePlugin/build-win/clap/x64/Release/MangrovePlugin.clap
%LOCALAPPDATA%\Programs\Common\CLAP\MangrovePlugin.clap   (installed by postbuild)
```

### Verify

The DLL must export `clap_entry`:

```powershell
dumpbin /EXPORTS MangrovePlugin\build-win\clap\x64\Release\MangrovePlugin.clap
```

The descriptor the host sees is assembled in `IPlug_include_in_plug_src.h` from
`MangrovePlugin/config.h`:

| Field | Value | Source |
|---|---|---|
| id | `com.Nassau.Mangrove` | `BUNDLE_DOMAIN.BUNDLE_MFR.BUNDLE_NAME` |
| name / vendor | `Mangrove` / `Nassau` | `PLUG_NAME` / `PLUG_MFR` |
| version | `5.0.0` | `PLUG_VERSION_STR` |
| features | `audio-effect`, `compressor` | `CLAP_FEATURES` |
| description | Two-stage level and density compressor | `CLAP_DESCRIPTION` |

The **id is the plugin's permanent identity** in host project files — changing it later
breaks saved sessions.

For deeper validation use [clap-validator](https://github.com/free-audio/clap-validator)
(`clap-validator validate MangrovePlugin.clap`), or load the plugin in Reaper/Bitwig.

### Note for anyone touching the plugin class

Constructor member-initialisers must name the base as **`iplug::Plugin`**, fully qualified.
Under CLAP, `IPlugCLAP` inherits `clap::helpers::Plugin`, whose injected-class-name shadows
`iplug::Plugin` inside the derived class — an unqualified `Plugin(info, ...)` compiles fine
for VST3 but fails for CLAP with *"Plugin is neither base nor member"*. The iPlug2 examples
qualify it for this reason.

---

## Testing Checklist

Print this and tick off each step:

- [ ] **Phase 0:** Environment validated (VS, Git, CMake optional)
- [ ] **Phase 1:** Repository cloned with submodules initialized
- [ ] **Phase 2:** `MangrovePlugin.sln` opened (not the `.vcxproj`)
- [ ] **Phase 3:** Project wiring verified — no manual edits needed
- [ ] **Phase 4:** Configuration set to Release | x64
- [ ] **Phase 5:** Build succeeded (0 failed)
- [ ] **Phase 6:** Bundle found at `MangrovePlugin\build-win\MangrovePlugin.vst3\`
- [ ] **Phase 7:** Plugin present in `%LOCALAPPDATA%\Programs\Common\VST3\`
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
│   │   ├── compressor_chain.cpp      ← compiled into the Windows plugin
│   │   └── compressor_chain.h
│   ├── Plugin/                       ← UI + the CMake (macOS) target
│   │   ├── MangroveUI.cpp            ← compiled into the Windows plugin
│   │   ├── MangroveUI.h
│   │   ├── MangrovePlugin.cpp        ← NOT used by the Windows build
│   │   ├── config.h                  ← stale copy; see duplicate-config trap (Phase 9)
│   │   └── CMakeLists.txt            ← macOS-only target
│   └── VST3/                         ← raw VST3-SDK plugin (separate, no IPlug2)
│
├── MangrovePlugin/                   ← the Windows build lives here
│   ├── MangrovePlugin.sln                 ← BUILD THIS (Phase 2)
│   ├── projects/
│   │   ├── MangrovePlugin-vst3.vcxproj    ← do not build directly (MSB4019)
│   │   ├── MangrovePlugin-clap.vcxproj
│   │   └── MangrovePlugin-aax.vcxproj
│   ├── build-win/                         ← output tree (Phase 6)
│   │   ├── vst3\x64\Release\MangrovePlugin.vst3   (loose DLL)
│   │   └── MangrovePlugin.vst3/                   (assembled bundle)
│   ├── config/
│   │   └── MangrovePlugin-win.props       ← include paths + graphics backend
│   ├── scripts/
│   │   └── postbuild-win.bat              ← builds bundle, installs to %LOCALAPPDATA%
│   ├── MangrovePlugin.cpp                 ← the plugin class used on Windows
│   ├── MangrovePlugin.h
│   └── config.h                           ← PLUG_N_PARAMS, PLUG_HAS_UI, sizes
│
├── external/
│   ├── vst3sdk/                      ← Steinberg VST3 SDK (used by Source/VST3 + CMake)
│   └── iplug2/                       ← IPlug2 framework; vendors its own VST3_SDK copy
│
├── BUILD.md                          ← root CMake guide — macOS only
└── docs/
    ├── BUILDING.md                   ← general/macOS build guide
    └── BUILDING_WIN11.md             ← THIS FILE
```

---

## Support & Troubleshooting Resources

- **IPlug2 Documentation:** https://github.com/iPlug2/iPlug2/wiki
- **VST3 SDK:** https://github.com/steinbergmedia/vst3sdk
- **Windows Audio Dev:** https://docs.microsoft.com/en-us/windows/win32/audio/core-audio-interfaces
- **This repo issues:** Report build errors at https://github.com/tweggen/nassau-mangrove2/issues

---

**Last Updated:** 2026-08-15  
**Tested on:** Windows 11 x64 — Visual Studio 18 (MSVC 14.51) and Visual Studio 2022 (v17.x);
VST3 Release|x64 built with the custom IGraphics UI  
**Contact:** Timo Weggen (timo.weggen@gmail.com)
