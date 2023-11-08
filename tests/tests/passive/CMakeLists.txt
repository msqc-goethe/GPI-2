cmake_minimum_required(VERSION 3.5)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")
set(APPS "passive" "passive_limits" "passive_limits_config" "passive_n" "passive_nbytes" "passive_own" "passive_own_block" "passive_queue_max_config" "passive_recvtimeout" "passive_tout_test")

foreach(APP IN LISTS APPS)
	if(APP STREQUAL "")
		continue()
	else()
		add_executable(${APP}.bin "${APP}.c")
		target_link_libraries(${APP}.bin PUBLIC "${GPI2_LIB}" "test_utils")
		target_include_directories(${APP}.bin PUBLIC "../")
	endif()
endforeach()
