add_library(compressor_chain
    compressor_chain.cpp
    compressor_chain.h
)

target_include_directories(compressor_chain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# ${NASSAU_WARNING_FLAGS}, not the GCC spelling. cmake/NassauSDK.cmake and the
# SDK's NassauPluginProject.cmake both set it per compiler -- /W4 on MSVC,
# -Wall -Wextra -Wpedantic elsewhere -- and this file previously spelled the
# GCC flags directly. cl(1) reads /Wextra as /W<n> and refuses it:
#
#     cl : command line error D8021: invalid numeric argument '/Wextra'
#
# which broke every MSVC build of this repo. The variable exists exactly so
# this file does not have to know which compiler it is.
target_compile_options(compressor_chain PRIVATE ${NASSAU_WARNING_FLAGS})
