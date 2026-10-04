# Mangrove plugin bundles (thin wrapper over the Nassau SDK -> iPlug2).
#
# One call emits VST3 (.vst3), AU v2 (.component) and CLAP (.clap) from the same
# sources, via iPlug2's official iplug_add_plugin(). The SDK picks the UI
# backend: prebuilt Skia + Metal when available, otherwise headless. Pass
# -DNASSAU_FORCE_HEADLESS=ON to force the no-UI build.
#
# WHAT THIS REPLACED: ~215 lines that declared iplug2_lib, MangroveIPlug_VST3
# and MangroveIPlug_AU by hand, listing iPlug2's own source files, include
# directories and per-format compile definitions. Its header comment explained
# the choice as "We do NOT use IPlug2's own CMake because it is Xcode/VS-primary"
# — true when it was written, and no longer: the SDK drives iPlug2's CMake on
# macOS and Windows for three other plugins.
#
# The target KEEPS THE NAME MangroveIPlug on purpose, even though the "IPlug"
# suffix only ever existed to tell this target apart from the Source/VST3 one
# that is now gone. Renaming it would change the bundle filename and the
# CFBundleExecutable that resources/MangroveIPlug-*-Info.plist declare, which is
# a product-visible change and not this migration's business. The plugin's
# DISPLAY name is unaffected either way: that comes from config.h's
# PLUG_NAME ("Mangrove"), as does the VST3 uid, so hosts see exactly what they
# saw before.
#
# config.h lives here; the Info.plist templates moved into resources/ with the
# <NAME>-<FORMAT>-Info.plist names iPlug2's AUv2/VST3/CLAP modules look for.

# AU IS DEDUCTED FROM THIS LIST ON PURPOSE. Mangrove's Audio Unit builds and
# registers, but its EDITOR CRASHES THE HOST: macOS's own auval segfaults
# reproducibly at "VERIFYING CUSTOM UI" (exit 139, 2/2 runs), where
# nassau-eq, nassau-zermatt and nassau-analogue all reach the same stage and
# report AU VALIDATION SUCCEEDED.
#
# This is not a regression from the SDK migration. Mangrove's AU target had
# never been built at all -- it sat behind `if(FALSE)` with the note
# "Temporarily disabled - VST3 is the priority for Phase 5" -- so the first
# time the format existed was the first time the defect could be seen, and the
# suite's packaging gate is what saw it.
#
# A plugin format whose editor crashes the host must not ship, so VST3 and CLAP
# go out and AU waits for an investigation. Put AU back in this list the moment
# `auval -v aufx Mng5 Nss2` exits 0.
#
# The two Info.plist bugs that had to be fixed before the crash was even
# reachable are fixed in resources/MangroveIPlug-AU-Info.plist and are kept,
# because they are prerequisites for AU ever working and both are verified.
nassau_add_plugin(MangroveIPlug
    FORMATS VST3 CLAP
    SOURCES MangrovePlugin.cpp MangroveUI.cpp
    LINK    compressor_chain)
