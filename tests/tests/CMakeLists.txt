cmake_minimum_required(VERSION 3.5)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")

add_library(test_utils "test_utils.c")
target_link_libraries(test_utils PUBLIC "${GPI2_LIB}")
target_include_directories(
  test_utils PUBLIC "./" "${CMAKE_SOURCE_DIR}/src/include"
)

add_subdirectory(atomic)
add_subdirectory(cm)
add_subdirectory(collectives)
add_subdirectory(comm)
add_subdirectory(config)
add_subdirectory(error_vec)
add_subdirectory(extensions)
add_subdirectory(groups)
add_subdirectory(init)
add_subdirectory(lists)
add_subdirectory(notifications)
add_subdirectory(passive)
add_subdirectory(segments)
