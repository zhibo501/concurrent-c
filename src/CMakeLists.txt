cmake_minimum_required (VERSION 2.8.2)

if (__CONS_PATCH__)
set (CONS_VISIBALE )
else ()
set (CONS_VISIBALE "-fvisibility=hidden")
endif ()

if (${CMAKE_C_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_C_COMPILER_ID} MATCHES "GNU")
	add_compile_options(-Werror)
	set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   -fprofile-arcs -ftest-coverage")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing ${CONS_VISIBALE}")
endif ()


aux_source_directory(. rcu_LIB_SRCS)

add_library(rcu SHARED ${rcu_LIB_SRCS})
target_include_directories(rcu PUBLIC include)
target_compile_options(rcu PRIVATE -std=c99)
target_link_libraries(rcu PRIVATE pthread)

install (TARGETS rcu DESTINATION lib)
install (FILES rcu.h DESTINATION include)
