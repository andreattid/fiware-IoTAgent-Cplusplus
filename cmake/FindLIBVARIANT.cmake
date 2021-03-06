#
# Find the Variant includes and client library
# This module defines
# LIBVARIANT_INCLUDE_DIR, where to find Variant/VariantDefines.h
# LIBVARIANT_LIBRARIES,
# LIBVARIANT_FOUND, If false, do not try to use libVariant.
# LIBVARIANT_LIBS_DIR
#
# var environment LIBVARIANT_ROOT

find_path(LIBVARIANT_INCLUDE_DIR Variant/VariantDefines.h
               ${IOTAGENT_DEPENDENCIES_PATHS}
               $ENV{LIBVARIANT_ROOT}/lib
               ${LIBVARIANT_ROOT}/lib)

set(LIBVARIANT_LIB_NAME "libVariant.a")
find_library(LIBVARIANT_LIBRARIES NAMES ${LIBVARIANT_LIB_NAME}
               PATHS
               ${IOTAGENT_DEPENDENCIES_PATHS}
               $ENV{LIBVARIANT_ROOT}/lib
               ${LIBVARIANT_ROOT}/lib
               NO_DEFAULT_PATH
)
if(LIBVARIANT_INCLUDE_DIR AND LIBVARIANT_LIBRARIES)
 get_filename_component(LIBVARIANT_LIBS_DIR ${LIBVARIANT_LIBRARIES} DIRECTORY)
 set(LIBVARIANT_LIBRARIES "${LIBVARIANT_LIBRARIES}")

 set(LIBVARIANT_FOUND TRUE)
else(LIBVARIANT_INCLUDE_DIR AND LIBVARIANT_LIBRARIES)
 set(LIBVARIANT_FOUND FALSE)
 if (LIBVARIANT_FIND_REQUIRED)
  message(FATAL_ERROR "LIBVARIANT not found.")
 else (LIBVARIANT_FIND_REQUIRED)
message(STATUS "LIBVARIANT not found.")
endif (LIBVARIANT_FIND_REQUIRED)
endif(LIBVARIANT_INCLUDE_DIR AND LIBVARIANT_LIBRARIES)
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MongoDB DEFAULT_MSG MongoDB_LIBRARIES MongoDB_INCLUDE_DIR MongoDB_LIBS_DIR)
mark_as_advanced(LIBVARIANT_INCLUDE_DIR LIBVARIANT_LIBRARIES LIBVARIANT_LIBS_DIR)
