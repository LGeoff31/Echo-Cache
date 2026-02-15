add_subdirectory(cache)

add_executable(worker worker.cpp)

find_library(libs NAMES Parser LocalCache SocketServer)
target_link_libraries(worker PRIVATE Parser LocalCache SocketServer)
