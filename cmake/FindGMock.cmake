# Find the Google Mock headers and libraries
# GMOCK_INCLUDE_DIR where to find gmock.h
find_path ( GMOCK_INCLUDE_DIR gmock/gmock.h
${IOTAGENT_DEPENDENCIES_PATHS}
NO_DEFAULT_PATH )
find_library(GMOCK_LIBRARY
	NAMES gmock
	PATHS ${IOTAGENT_DEPENDENCIES_PATHS} 
	NO_DEFAULT_PATH)
if (GMOCK_LIBRARY)
  get_filename_component(GMOCK_LIBS_DIR ${GMOCK_LIBRARY} DIRECTORY)
	set(GMOCK_LIBRARIES "gmock gtest")
	set(GMOCK_FOUND TRUE)
endif()
include ( EmbeddedGTest )
include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( GMOCK DEFAULT_MSG GMOCK_INCLUDE_DIR
GMOCK_LIBS_DIR
GMOCK_LIBRARIES
)
mark_as_advanced ( GMOCK_INCLUDE_DIR GMOCK_LIBRARIES GMOCK_LIBS_DIR )
