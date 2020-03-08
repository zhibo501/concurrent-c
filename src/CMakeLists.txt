cmake_minimum_required (VERSION 2.8.2)

aux_source_directory(. rcu_LIB_SRCS)

add_library(rcu SHARED ${rcu_LIB_SRCS})
target_include_directories(rcu PUBLIC include)

install (TARGETS rcu DESTINATION lib)
install (FILES rcu.h DESTINATION include)
