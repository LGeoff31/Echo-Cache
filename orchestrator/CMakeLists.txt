add_executable(orchestrator orchestrator.cpp helpers.cpp)
find_library(libs NAMES Parser SocketClient SocketServer)
target_link_libraries(orchestrator PRIVATE Parser SocketClient SocketServer)
