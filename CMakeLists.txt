cmake_minimum_required (VERSION 2.8.2)
project (concurrent)

# The version number.
set (Concurrent_VERSION_MAJOR 0)
set (Concurrent_VERSION_MINOR 1)
set (Concurrent_VERSION_PATCH 0)
if (NOT Concurrent_VERSION_REVISION)
	message(STATUS "Concurrent-c revision not set.")
	set (Concurrent_VERSION_REVISION "unknown")

	find_package(Git)
	if (GIT_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
			OUTPUT_VARIABLE Concurrent_VERSION_REVISION
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	endif ()
endif ()

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "CMAKE_BULD_TYPE not set. use Release as default.")
	set(CMAKE_BUILD_TYPE "Release")
endif ()


################################  cpu arch ################################

if (NOT (${CMAKE_SYSTEM_PROCESSOR} MATCHES ${CMAKE_HOST_SYSTEM_PROCESSOR}))
	message(FATAL_ERROR "concurrent-c build fail. error: Cross compile NOT SUPPORT.")
endif ()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
	message(STATUS "Now in x86_64 architect ...")
	add_definitions(
		-D_CPU_TYPE=0
		-D_HARDWARE_PLATFORM=0
		-D_WORDSIZE=64
	)
	if (${CMAKE_C_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_C_COMPILER_ID} MATCHES "GNU")
		add_compile_options(-m64)
	endif ()
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
	message(STATUS "Now in aarch64 architect ...")
	add_definitions(
		-D_CPU_TYPE=49
		-D_HARDWARE_PLATFORM=8
		-D_WORDSIZE=64
	)
else ()
	message(FATAL_ERROR "Concurrent-c build fail. error: NOT SUPPORT architect for " ${CMAKE_SYSTEM_PROCESSOR})
endif ()


################################  os  ################################

if (WIN32)
    message(STATUS "Now is Windows")
endif ()
if (UNIX)
    message(STATUS "Now is UNIX-like OS's. Including Apple OS X  and CygWin")
    add_definitions(
    	-DLINUX
    )
endif ()
if (APPLE)
    message(STATUS "Now is Apple systems.")
    set(CMAKE_MACOSX_RPATH 1)
endif ()


################################  compiler  ################################

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
  # using regular Clang or AppleClang
	message(STATUS "Now is clang ...")
	add_compile_options(-Wall -g -fPIC -fstack-protector-all)
endif()

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
	message(STATUS "Now is gnu cc ...")
	add_compile_options(-Wall -g -fPIC -fstack-protector-all)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro,-z,now,-z,noexecstack")
	add_definitions(
		-D_GNU_SOURCE
	)
endif ()


################################  public  ################################

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

enable_testing()
add_subdirectory(tests)
