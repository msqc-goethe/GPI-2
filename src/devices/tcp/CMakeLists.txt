cmake_minimum_required(VERSION 3.5)

add_library(
  tcp-device SHARED
  "GPI2_TCP.c"
  "GPI2_TCP_ATOMIC.c"
  "GPI2_TCP_GRP.c"
  "GPI2_TCP_IO.c"
  "GPI2_TCP_PASSIVE.c"
  "GPI2_TCP_SEG.c"
  "list.c"
  "rb.c"
  "tcp_device.c"
)

target_compile_features(tcp-device PUBLIC c_std_11)
target_compile_definitions(tcp-device PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(
  tcp-device PUBLIC "../include" "${CMAKE_SOURCE_DIR}/src"
                    "${CMAKE_SOURCE_DIR}/src/include"
)
