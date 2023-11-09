cmake_minimum_required(VERSION 3.5)

find_package(Threads REQUIRED)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")
set(APPS
    "all-to-all"
    "all-to-rank0"
    "comm_limits"
    "comm_transfer_max_config"
    "comm_write_m_to_1"
    "queue_full"
    "read_nsizes"
    "read_smalls"
    "read_write"
    "read_zero_size"
    "small_writes_notify"
    "strings"
    "write"
    "write_all"
    "write_all_mtt"
    "write_all_nsizes"
    "write_all_nsizes_nobuild"
    "write_right_left"
    "write_simple"
    "write_timeout"
    "write_zero_size"
    "z4k_pressure"
    "z4k_pressure_mtt"
)

foreach(APP IN LISTS APPS)
  if(APP STREQUAL "")
    continue()
  else()
    add_executable(${APP}.bin "${APP}.c")
    target_link_libraries(
      ${APP}.bin PUBLIC "${GPI2_LIB}" "test_utils" "Threads::Threads"
    )
    target_include_directories(${APP}.bin PUBLIC "../")
  endif()
endforeach()
