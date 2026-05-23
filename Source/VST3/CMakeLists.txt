# Mangrove VST3 Plugin CMakeLists.txt
# Builds IPlug2 + VST3 SDK as static libraries, then the plugin bundle

set(IPLUG_ROOT "${CMAKE_SOURCE_DIR}/external/iPlug2")
set(VST3_SDK_ROOT "${IPLUG_ROOT}/Dependencies/IPlug/VST3_SDK")
set(WDL_ROOT "${IPLUG_ROOT}/WDL")

# ── 1. IPlug2 static library ──────────────────────────────────────────────────
# Compile IPlug2 core + VST3 adapter as a static lib.
# We do NOT use IPlug2's own CMake because it is Xcode/VS-primary.

add_library(iplug2_vst3 STATIC
    ${IPLUG_ROOT}/IPlug/IPlugAPIBase.cpp
    ${IPLUG_ROOT}/IPlug/IPlugParameter.cpp
    ${IPLUG_ROOT}/IPlug/IPlugPluginBase.cpp
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

# ── 2. VST3 SDK base library (required by IPlug2 VST3 adapter) ────────────────
# The VST3 SDK ships its own CMakeLists.txt but it often conflicts.
# Safer: compile the handful of required files directly.

add_library(vst3_sdk_base STATIC
    ${VST3_SDK_ROOT}/base/source/baseiids.cpp
    ${VST3_SDK_ROOT}/base/source/fbuffer.cpp
    ${VST3_SDK_ROOT}/base/source/fobject.cpp
    ${VST3_SDK_ROOT}/base/source/fstring.cpp
    ${VST3_SDK_ROOT}/base/source/fstreamer.cpp
    ${VST3_SDK_ROOT}/base/source/updatehandler.cpp
    ${VST3_SDK_ROOT}/pluginterfaces/base/coreiids.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/common/commoniids.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/common/pluginview.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstaudioeffect.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstbus.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstcomponent.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstcomponentbase.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vsteditcontroller.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstinitiids.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstparameters.cpp
    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstsinglecomponenteffect.cpp
)

target_compile_options(vst3_sdk_base PRIVATE -w)

target_include_directories(vst3_sdk_base PUBLIC
    ${VST3_SDK_ROOT}
    ${VST3_SDK_ROOT}/pluginterfaces
    ${VST3_SDK_ROOT}/base
    ${VST3_SDK_ROOT}/public.sdk/source/vst
)

target_link_libraries(iplug2_vst3 PUBLIC vst3_sdk_base)

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
