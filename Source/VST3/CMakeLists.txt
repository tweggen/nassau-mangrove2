# Mangrove VST3 Plugin CMakeLists.txt
# Builds IPlug2 + VST3 SDK as static libraries, then the plugin bundle

set(IPLUG_ROOT "${CMAKE_SOURCE_DIR}/external/iplug2")
set(VST3_SDK_ROOT "${IPLUG_ROOT}/Dependencies/IPlug/VST3_SDK")
set(WDL_ROOT "${IPLUG_ROOT}/WDL")

# ── 1. IPlug2 static library ──────────────────────────────────────────────────
# Compile IPlug2 core + VST3 adapter as a static lib.
# We do NOT use IPlug2's own CMake because it is Xcode/VS-primary.

add_library(iplug2_vst3 STATIC
    ${IPLUG_ROOT}/IPlug/IPlugAPIBase.cpp
    ${IPLUG_ROOT}/IPlug/IPlugParameter.cpp
    ${IPLUG_ROOT}/IPlug/IPlugPluginBase.cpp
    ${IPLUG_ROOT}/IPlug/IPlugProcessor.cpp
    ${IPLUG_ROOT}/IPlug/IPlugTimer.cpp
    ${IPLUG_ROOT}/IPlug/VST3/IPlugVST3.cpp
    ${IPLUG_ROOT}/IPlug/VST3/IPlugVST3_Controller.cpp
    ${IPLUG_ROOT}/IPlug/VST3/IPlugVST3_Processor.cpp
    ${IPLUG_ROOT}/IPlug/VST3/IPlugVST3_ProcessorBase.cpp
    ${IPLUG_ROOT}/IPlug/VST3/IPlugVST3_View.h
)

# Suppress IPlug2 third-party warnings
target_compile_options(iplug2_vst3 PRIVATE -w)

target_include_directories(iplug2_vst3 PUBLIC
    ${IPLUG_ROOT}/IPlug
    ${IPLUG_ROOT}/IPlug/VST3
    ${IPLUG_ROOT}/IPlug/Extras
    ${IPLUG_ROOT}/IGraphics
    ${IPLUG_ROOT}/WDL
    ${VST3_SDK_ROOT}
    ${VST3_SDK_ROOT}/pluginterfaces
    ${VST3_SDK_ROOT}/base
    ${VST3_SDK_ROOT}/public.sdk/source/vst
    ${VST3_SDK_ROOT}/public.sdk/source/common
)

# IPlug2's no-GUI configuration flags
target_compile_definitions(iplug2_vst3 PUBLIC
    IPLUG_DSP=1
    PLUG_HAS_UI=0
    IPLUG_EDITOR=0
    IGRAPHICS_NANOVG=0
    IGRAPHICS_NONE=1
    NO_IGRAPHICS=1
    VST3_API=1
)

# ── 2. Link against main VST3 SDK targets (configured in root CMakeLists) ────
# The root CMakeLists.txt already configures and builds the VST3 SDK
# We reuse those targets (sdk, pluginterfaces) instead of rebuilding

target_link_libraries(iplug2_vst3 PUBLIC sdk pluginterfaces)

# ── 3. Plugin source files ────────────────────────────────────────────────────
add_library(MangroveVST3 MODULE
    MangrovePlugin.cpp
)

set_target_properties(MangroveVST3 PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "vst3"
    OUTPUT_NAME "Mangrove"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.nassau.mangrove"
    MACOSX_BUNDLE_BUNDLE_VERSION "4.0.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "4.0"
)

target_include_directories(MangroveVST3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Source/DSP
)

target_link_libraries(MangroveVST3 PRIVATE
    iplug2_vst3
    sdk
    pluginterfaces
    compressor_chain
    "-framework CoreFoundation"
    "-framework AudioUnit"
    "-framework AudioToolbox"
    "-framework CoreAudio"
)

# Install to user VST3 folder for DAW testing
install(TARGETS MangroveVST3
    LIBRARY DESTINATION "Library/Audio/Plug-Ins/VST3"
)
