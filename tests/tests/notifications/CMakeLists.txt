cmake_minimum_required(VERSION 3.5)

find_package(Threads REQUIRED)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")
set(APPS
    "not_zero_wait"
    "notification_value"
    "notify_after_delete"
    "notify_all"
    "notify_last"
    "notify_null"
    "read_notify"
    "read_notify_zero_size"
    "write_m_to_1"
    "write_notify"
    "write_notify_outstanding"
    "write_notify_zero_size"
)

foreach(APP IN LISTS APPS)
  if(APP STREQUAL "")
    continue()
  else()
    if(APP STREQUAL "notification_value")
      add_executable(${APP}.bin "${APP}.cpp")
    else()
      add_executable(${APP}.bin "${APP}.c")
    endif()
    target_link_libraries(
      ${APP}.bin PUBLIC "${GPI2_LIB}" "test_utils" "Threads::Threads"
    )
    target_include_directories(${APP}.bin PUBLIC "../")
  endif()
endforeach()
