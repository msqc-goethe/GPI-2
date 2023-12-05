cmake_minimum_required(VERSION 3.5)

if(WITH_IB)
	add_subdirectory(ib)
elseif(WITH_PORTALS)
	add_subdirectory(portals)
else()
	add_subdirectory(tcp)
endif()
