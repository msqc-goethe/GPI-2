cmake_minimum_required(VERSION 3.5)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")
set(APPS
    "g_before_start"
    "g_coll_del_coll"
    "g_commit_g_all"
    "g_commit_try_block"
    "g_elems"
    "g_force_timeout"
    "g_max_groups"
    "g_max_groups_config"
    "g_no_own"
    "g_num"
    "g_size"
    "g_some_from_all"
    "g_with_one_hole"
    "g_with_two_holes"
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
