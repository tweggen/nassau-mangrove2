cmake_minimum_required(VERSION 3.15)

set(IPLUG2_ROOT "${CMAKE_SOURCE_DIR}/external/iplug2")
set(IPLUG2_IPLUG    "${IPLUG2_ROOT}/IPlug")
set(IPLUG2_IGRAPHICS "${IPLUG2_ROOT}/IGraphics")
set(IPLUG2_DEPS     "${IPLUG2_ROOT}/Dependencies")
set(NANOVG_DIR      "${IPLUG2_DEPS}/IGraphics/NanoVG/src")
set(NANOSVG_DIR     "${IPLUG2_DEPS}/IGraphics/NanoSVG/src")
set(METALNVG_DIR    "${IPLUG2_DEPS}/IGraphics/MetalNanoVG/src")
set(STB_DIR         "${IPLUG2_DEPS}/IGraphics/STB")

# ── IPlug2 common static library (DSP-only, no graphics) ────────────────────
add_library(iplug2_lib STATIC
    ${IPLUG2_IPLUG}/IPlugAPIBase.cpp
    ${IPLUG2_IPLUG}/IPlugParameter.cpp
    ${IPLUG2_IPLUG}/IPlugPaths.cpp
    ${IPLUG2_IPLUG}/IPlugPluginBase.cpp
    ${IPLUG2_IPLUG}/IPlugProcessor.cpp
    ${IPLUG2_IPLUG}/IPlugTimer.cpp
)
target_compile_options(iplug2_lib PRIVATE -w)
target_compile_definitions(iplug2_lib PUBLIC
)
target_include_directories(iplug2_lib PUBLIC
    ${IPLUG2_IPLUG}
    ${IPLUG2_IPLUG}/Extras
    ${IPLUG2_IGRAPHICS}
    ${IPLUG2_IGRAPHICS}/Controls
    ${IPLUG2_IGRAPHICS}/Platforms
    ${IPLUG2_IGRAPHICS}/Drawing
    ${IPLUG2_ROOT}/WDL
    ${NANOVG_DIR}
    ${NANOSVG_DIR}
    ${METALNVG_DIR}
    ${STB_DIR}
    ${IPLUG2_DEPS}/IGraphics/glad/include
)

# ── VST3 plugin bundle (DSP-only, no GUI) ────────────────────────────────────
add_library(MangroveIPlug_VST3 MODULE
    MangrovePlugin.cpp
    ${IPLUG2_IPLUG}/VST3/IPlugVST3.cpp
    ${IPLUG2_IPLUG}/VST3/IPlugVST3_Controller.cpp
    ${IPLUG2_IPLUG}/VST3/IPlugVST3_Processor.cpp
    ${IPLUG2_IPLUG}/VST3/IPlugVST3_ProcessorBase.cpp
    ${CMAKE_SOURCE_DIR}/external/vst3sdk/public.sdk/source/vst/vstsinglecomponenteffect.cpp
    ${CMAKE_SOURCE_DIR}/external/vst3sdk/public.sdk/source/main/pluginfactory.cpp
    ${CMAKE_SOURCE_DIR}/external/vst3sdk/public.sdk/source/main/macmain.cpp
)
target_compile_options(MangroveIPlug_VST3 PRIVATE -Wall -Wextra)
target_compile_definitions(MangroveIPlug_VST3 PRIVATE VST3_API=1 IPLUG_EDITOR=0 NO_IGRAPHICS=1 IGRAPHICS_NONE=1)
target_include_directories(MangroveIPlug_VST3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Source/DSP
    ${IPLUG2_IPLUG}
    ${IPLUG2_IPLUG}/VST3
    ${IPLUG2_IGRAPHICS}
    ${CMAKE_SOURCE_DIR}/external/vst3sdk
    ${CMAKE_SOURCE_DIR}/external/vst3sdk/pluginterfaces
    ${CMAKE_SOURCE_DIR}/external/vst3sdk/public.sdk
    ${CMAKE_SOURCE_DIR}/external/vst3sdk/base
)
target_link_libraries(MangroveIPlug_VST3 PRIVATE
    iplug2_lib compressor_chain
    sdk pluginterfaces
    "-framework Cocoa"
    "-framework Metal" "-framework MetalKit" "-framework QuartzCore"
    "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreMIDI"
    "-framework Accelerate"
)
set_target_properties(MangroveIPlug_VST3 PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "vst3"
    OUTPUT_NAME "MangroveIPlug"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.nassau.mangrove.iplug"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.vst3"
)

# ── AUv2 plugin bundle (DSP-only, no GUI) ───────────────────────────────────
if(APPLE)
    add_library(MangroveIPlug_AU MODULE
        MangrovePlugin.cpp
        ${IPLUG2_IPLUG}/AUv2/IPlugAU.cpp
        ${IPLUG2_IPLUG}/AUv2/IPlugAU_view_factory.mm
    )
    target_compile_options(MangroveIPlug_AU PRIVATE -Wall -Wextra -w)
    target_compile_definitions(MangroveIPlug_AU PRIVATE AU_API=1 IPLUG_EDITOR=0)
    target_include_directories(MangroveIPlug_AU PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Source/DSP
        ${IPLUG2_IPLUG}
        ${IPLUG2_IPLUG}/AUv2
        ${IPLUG2_IGRAPHICS}
    )
    target_link_libraries(MangroveIPlug_AU PRIVATE
        iplug2_lib compressor_chain
        "-framework Cocoa" "-framework AudioUnit" "-framework CoreAudio"
        "-framework AudioToolbox" "-framework CoreMIDI"
        "-framework Metal" "-framework MetalKit" "-framework QuartzCore"
        "-framework Carbon" "-framework Accelerate"
    )
    set_target_properties(MangroveIPlug_AU PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION "component"
        OUTPUT_NAME "MangroveIPlug"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.nassau.mangrove.iplug.au"
    )
endif()
