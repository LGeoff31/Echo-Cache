add_executable(main client.cpp)

find_library(libs NAMES CppClient Parser)
target_link_libraries(main PRIVATE CppClient Parser)