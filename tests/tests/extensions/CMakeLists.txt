cmake_minimum_required(VERSION 3.5)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")
set(APPS "ping_procs" "q_create_delete_wait" "q_create_use_queue"
         "seg_avail_local" "seg_avail_local_usermsegs"
)

foreach(APP IN LISTS APPS)
  if(APP STREQUAL "")
    continue()
  else()
    add_executable(${APP}.bin "${APP}.c")
    target_link_libraries(${APP}.bin PUBLIC "${GPI2_LIB}" "test_utils")
    target_include_directories(${APP}.bin PUBLIC "../")
  endif()
endforeach()
