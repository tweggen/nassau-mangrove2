# VST3 Plugin Factory Registration Fix

## Problem
The Mangrove VST3 plugin was being scanned by DAWs (e.g., PreSonus Studio One) but not appearing in the plugin list. The plugin cache showed `numClasses="0"`, indicating the VST3 factory was created but empty.

## Root Cause
Two critical VST3 SDK files were missing from the CMake build configuration:
1. `pluginfactory.cpp` - Creates and manages the VST3 factory, registers plugin classes
2. `macmain.cpp` - macOS entry point (bundleEntry/bundleExit) that initializes the factory module

Without these files:
- The factory object was instantiated but never populated with plugin classes
- DAWs could load the plugin bundle but found no classes to register
- The plugin was effectively invisible to DAW plugin lists

## Solution
Added the two missing VST3 SDK files to the Source/VST3/CMakeLists.txt build target:
```cmake
add_library(MangroveVST3 MODULE
    MangrovePlugin.cpp
    "${CMAKE_SOURCE_DIR}/external/vst3sdk/public.sdk/source/vst/vstsinglecomponenteffect.cpp"
    "${CMAKE_SOURCE_DIR}/external/vst3sdk/public.sdk/source/main/pluginfactory.cpp"
    "${CMAKE_SOURCE_DIR}/external/vst3sdk/public.sdk/source/main/macmain.cpp"
)
```

## Changes Made
1. **Source/VST3/CMakeLists.txt**: Added pluginfactory.cpp and macmain.cpp to build
2. **CMakeLists.txt**: Temporarily disabled Source/Plugin build due to graphics compilation conflicts
3. **Source/Plugin/CMakeLists.txt**: Added compile flags to prevent graphics compilation errors
4. **Source/Plugin/config.h**: Reverted PLUG_HAS_UI to 0

## Verification
The rebuilt plugin binary now exports the GetPluginFactory symbol:
```bash
$ nm -gU Mangrove.vst3/Contents/MacOS/Mangrove | grep GetPluginFactory
00000000000031f0 T _GetPluginFactory
```

## Testing Instructions
1. **Code Signature**: Re-sign the plugin
   ```bash
   codesign -s - --force --deep ~/Library/Audio/Plug-Ins/VST3/Mangrove.vst3
   ```

2. **DAW Plugin Rescan**: Open PreSonus Studio One > Menu > Setup > Plug-in Manager > Rescan VST Plug-ins

3. **Expected Result**: 
   - Plugin should appear in the plugin list
   - Should be instantiable on a channel with correct parameters
   - DSP-only version (no UI yet, but plugin functional)

4. **Verify Factory**: Check PreSonus plugin cache to confirm `numClasses > 0`

## Next Steps
- Once factory registration is confirmed working, re-enable the Source/Plugin build with graphics support
- Fix the IGraphicsMac.h compilation issues (override keyword problem)
- Add UI implementation for Phase 5

## Build Information
- **Build Path**: `build_phase5/` 
- **Plugin Bundle**: `Source/VST3/Mangrove.vst3`
- **Installation**: `~/Library/Audio/Plug-Ins/VST3/Mangrove.vst3` (user folder)
- **System Folder**: `/Library/Audio/Plug-Ins/VST3/` (requires sudo)
