cmake_minimum_required(VERSION 3.5)

if(WITH_IB)
	add_subdirectory(ib)
elseif(WITH_BXI)
	add_subdirectory(bxi-portals)
else()
	add_subdirectory(tcp)
endif()
