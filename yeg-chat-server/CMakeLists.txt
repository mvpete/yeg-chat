cmake_minimum_required(VERSION 3.16)

project(yeg-chat-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++17 support has been enabled by default.")

find_package(Threads REQUIRED QUIET)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36.0 REQUIRED COMPONENTS system)

add_library(ChatServer chat_server.cpp)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(yeg-chat-server main.cpp)
target_link_libraries(yeg-chat-server ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ChatServer)

