find_path(
	PORTALS_INCLUDE_DIR
	NAMES "portals4.h"
	PATH_SUFFIXES "include"
)

find_library(PORTALS_LIBRARY
	NAMES "libportals.so"
	PATH_SUFFIXES "lib" "lib64"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Portals REQUIRED_VARS PORTALS_INCLUDE_DIR PORTALS_LIBRARY
)

if(Portals_FOUND)
	if(NOT TARGET Portals::Portals)
		add_library(Portals::Portals UNKNOWN IMPORTED GLOBAL)
		set_target_properties(Portals::Portals PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PORTALS_INCLUDE_DIR} IMPORTED_LOCATION ${PORTALS_LIBRARY})
	endif()
endif()

mark_as_advanced(PORTALS_INCLUDE_DIR PORTALS_LIBRARY)
