cmake_minimum_required(VERSION 3.5)
project(GPI-2)

option(WITH_IB "Build IB device" OFF)
option(WITH_BXI "Build BXI device" OFF)
option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

add_subdirectory(src)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
