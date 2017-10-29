# Look for JSONCPP if asked to.
# We use a bundled version by default because some distros ship versions of
# JSONCPP that cause segfaults and other memory errors when we link with them.
# See https://github.com/minetest/minetest/issues/1793

mark_as_advanced(JSON_LIBRARY JSON_INCLUDE_DIR)

find_library(JSON_LIBRARY NAMES jsoncpp)
find_path(JSON_INCLUDE_DIR json/features.h PATH_SUFFIXES jsoncpp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSONCPP DEFAULT_MSG JSON_LIBRARY JSON_INCLUDE_DIR)

if(JSONCPP_FOUND)
	message(STATUS "jsoncpp library found.")
endif()
