cmake_minimum_required (VERSION 2.8.2)
project (concurrent)

# The version number.
set (Concurrent_VERSION_MAJOR 0)
set (Concurrent_VERSION_MINOR 1)
set (Concurrent_VERSION_PATCH 0)

if (WIN32)
    message(STATUS "Now is Windows")
endif ()
if (UNIX)
    message(STATUS "Now is UNIX-like OS's. Including Apple OS X  and CygWin")
endif ()
if (APPLE)
    message(STATUS "Now is Apple systems.")
    set(CMAKE_MACOSX_RPATH 1)
endif ()

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
# add the binary tree to the search path for include files
# so that we will find 'config.h'
include_directories("${PROJECT_BINARY_DIR}")

# add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(cmd)

# enable_testing()
# add_subdirectory(tests)
