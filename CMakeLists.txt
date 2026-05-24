cmake_minimum_required(VERSION 3.15)
project(MangroveCompressor VERSION 4.0.0 LANGUAGES CXX C OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

# DSP Library
add_subdirectory(Source/DSP)

# VST3 Plugin (optional, depends on VST3 SDK)
set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/vst3sdk")
if(EXISTS "${VST3_SDK_ROOT}/CMakeLists.txt")
    set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "Disable VST3 examples")
    set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "Disable VST3 hosting examples")
    set(SMTG_ENABLE_VSTGUI_SUPPORT OFF CACHE BOOL "Disable VSTGUI")
    add_subdirectory(${VST3_SDK_ROOT} external/vst3sdk EXCLUDE_FROM_ALL)
    add_subdirectory(Source/VST3)
else()
    message(STATUS "VST3 SDK not found at ${VST3_SDK_ROOT} — skipping VST3 target")
endif()

# IPlug2 GUI Plugin (optional, depends on IPlug2)
# Temporarily disabled while fixing factory registration issue
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/iplug2/IPlug/IPlugAPIBase.h")
#     add_subdirectory(Source/Plugin)
# else()
#     message(STATUS "IPlug2 not found — skipping IPlug2 GUI plugin target")
# endif()

# Tests
enable_testing()
add_subdirectory(Tests)
