cmake_minimum_required(VERSION 3.5)

set(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.so")
set(APPS "list_limit" "list_ops_zero_size" "list_read" "list_read_notify" "list_read_notify_usermelems" "list_read_usermelems" "list_write" "list_write_notify" "list_write_notify_usermelems" "list_write_usermelems")

foreach(APP IN LISTS APPS)
	if(APP STREQUAL "")
		continue()
	else()
		add_executable(${APP}.bin "${APP}.c")
		target_link_libraries(${APP}.bin PUBLIC "${GPI2_LIB}" "test_utils")
		target_include_directories(${APP}.bin PUBLIC "../")
	endif()
endforeach()
