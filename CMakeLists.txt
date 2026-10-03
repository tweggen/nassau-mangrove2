cmake_minimum_required(VERSION 3.15)
project(MangroveCompressor VERSION 4.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate the shared Nassau plugin SDK and apply its project prologue: the
# OBJC/OBJCXX enablement, the macOS deployment target, NASSAU_WARNING_FLAGS /
# NASSAU_RELEASE_OPT_FLAGS, and the MSVC /MT runtime selection.
#
# This repo is the last of the four plugins to adopt the SDK, and it had the
# most to give up: its own vendored external/iplug2 AND external/vst3sdk
# submodules (9 GB on disk), its own VST3_SDK_ROOT, and a hand-rolled
# Source/VST3 target that compiled iPlug2's VST3 adapter itself. All three are
# gone; the SDK owns that layer now, and pins the same upstream commits this
# repo already used (iplug2 7dfe7a96d, vst3sdk 58f8da7), so no third-party code
# changed in the move.
#
# OBJCXX is no longer in the project() language list: it is enabled by the SDK
# prologue, on Apple only. Having it here unconditionally is what stopped this
# repo configuring on a non-Apple host at all.
#
# Two constraints the prologue documents and this call site honours: it must
# come BEFORE the first add_subdirectory() (CMAKE_MSVC_RUNTIME_LIBRARY seeds
# each target's property at creation time) and it must be included from THIS
# top-level scope (enable_language(OBJC) has to happen here for iPlug2).
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/NassauSDK.cmake)

# DSP core (always builds — no framework dependencies).
add_subdirectory(Source/DSP)

# Plugin bundles: VST3 + AU + CLAP on macOS, VST3 + CLAP on Windows, none
# elsewhere. CLAP is NEW here — the hand-rolled targets this replaces emitted
# VST3 and AU only, and one nassau_add_plugin() call gets the third format for
# free.
if(NASSAU_PLUGIN_TARGETS_POSSIBLE)
    include(${NASSAU_SDK_DIR}/cmake/NassauPlugin.cmake)
    add_subdirectory(Source/Plugin)
endif()

# Tests (always build).
enable_testing()
add_subdirectory(Tests)
