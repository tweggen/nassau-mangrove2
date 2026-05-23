add_executable(dsp_tests dsp_tests.cpp)
target_link_libraries(dsp_tests compressor_chain)
target_include_directories(dsp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Source/DSP)
target_compile_options(dsp_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME DSPTests COMMAND dsp_tests)
