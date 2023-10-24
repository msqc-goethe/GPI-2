find_path(
	PORTALS_INCLUDE_DIR
	NAMES "portals4.h" "portals4_bxiext.h"
	PATH_SUFFIXES "include"
)

find_library(PORTALS_LIBRARY
	NAMES libportals
	PATH_SUFFIXES "lib" "lib64"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Portals4 REQUIRED_VARS PORTALS_INCLUDE_DIR PORTALS_LIBRARY
)

if(Portals4_FOUND)
	if(NOT TARGET Portals4::BXI)
		add_library(Portals4::BXI)
		set_target_properties(Portals4::BXI PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PORTALS_INCLUDE_DIR} IMPORTED_LOCATION ${PORTALS_LIBRARY})
	endif()
endif()

mark_as_advanced(PORTALS_INCLUDE_DIR PORTALS_LIBRARY)
