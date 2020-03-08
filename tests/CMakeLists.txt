cmake_minimum_required (VERSION 2.8.2)

configure_file(CMakeLists.txt.in test-download/CMakeLists.txt)


execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test-download )
if(result)
  message(FATAL_ERROR "CMake step for test-tools failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test-download )
if(result)
  message(FATAL_ERROR "Build step for test-tools failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build)
#                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(rcu_test_bin rcu_testcase.cpp)
target_link_libraries(rcu_test_bin rcu gtest_main gmock_main)

if (CMAKE_VERSION VERSION_LESS 3.9.0)
add_test(rcu_run_case rcu_test_bin)
else()
include(GoogleTest)
gtest_add_tests(rcu_test_bin "" AUTO)
endif()

