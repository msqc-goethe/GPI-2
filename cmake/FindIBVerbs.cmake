find_path(
	IB_INCLUDE_DIR
	NAMES "verbs.h"
	PATH_SUFFIXES "infiniband"
)

find_library( IB_LIBRARY
	NAMES ibverbs
	PATH_SUFFIXES "lib" "lib64"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	IBVerbs REQUIRED_VARS IB_INCLUDE_DIR IB_LIBRARY
)

if(IBVerbs_FOUND)
	if(NOT TARGET IBVerbs::IBVerbs)
		add_library(IBVerbs::IBVerbs UNKNOWN IMPORTED GLOBAL)
		set_target_properties(IBVerbs::IBVerbs PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${IB_INCLUDE_DIR} IMPORTED_LOCATION ${IB_LIBRARY})
	endif()
endif()

mark_as_advanced(IB_INCLUDE_DIR IB_LIBRARY)
