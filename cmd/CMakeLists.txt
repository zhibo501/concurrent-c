cmake_minimum_required (VERSION 2.8.2)

add_executable(rcu-tool main.c)
target_link_libraries(rcu-tool rcu)

