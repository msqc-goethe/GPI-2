cmake_minimum_required(VERSION 3.5)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")
set(APPS "allreduce" "allreduce_user_fun" "allreduce_user_grp" "allreduce_user_grp_fun" "allreduce_user_type" "allreduce_utils" "allreduce_vlarge" "barrier" "barrier_grp" "barrier_timeout" "loop_barrier_group_timeout")

foreach(APP IN LISTS APPS)
	if(APP STREQUAL "")
		continue()
	else()
		add_executable(${APP}.bin "${APP}.c")
		target_link_libraries(${APP}.bin PUBLIC "${GPI2_LIB}" "test_utils")
		target_include_directories(${APP}.bin PUBLIC "../")
	endif()
endforeach()
