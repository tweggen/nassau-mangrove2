# CMake generated Testfile for 
# Source directory: /Users/tweggen/coding/github/nassau-mangrove2/Tests
# Build directory: /Users/tweggen/coding/github/nassau-mangrove2/build_xcode/Tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(DSPTests "/Users/tweggen/coding/github/nassau-mangrove2/build_xcode/Tests/Debug/dsp_tests")
  set_tests_properties(DSPTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;6;add_test;/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(DSPTests "/Users/tweggen/coding/github/nassau-mangrove2/build_xcode/Tests/Release/dsp_tests")
  set_tests_properties(DSPTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;6;add_test;/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(DSPTests "/Users/tweggen/coding/github/nassau-mangrove2/build_xcode/Tests/MinSizeRel/dsp_tests")
  set_tests_properties(DSPTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;6;add_test;/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(DSPTests "/Users/tweggen/coding/github/nassau-mangrove2/build_xcode/Tests/RelWithDebInfo/dsp_tests")
  set_tests_properties(DSPTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;6;add_test;/Users/tweggen/coding/github/nassau-mangrove2/Tests/CMakeLists.txt;0;")
else()
  add_test(DSPTests NOT_AVAILABLE)
endif()
