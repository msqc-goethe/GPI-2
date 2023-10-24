cmake_minimum_required(VERSION 3.5)
project(GPI-2)

option(WITH_IB "Build IB device" OFF)
option(WITH_BXI "Build BXI device" ON)
option(BUILD_TEST "Build tests" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

add_subdirectory(src)

if(BUILD_TEST)
	enable_testing()
	add_subdirectory(tests)
endif()
