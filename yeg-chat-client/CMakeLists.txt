cmake_minimum_required(VERSION 3.16)

project(yeg-chat-client)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++17 support has been enabled by default.")

find_package(Threads REQUIRED QUIET)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system)

if (UNIX)
  find_package(Curses REQUIRED)
  if(CURSES_FOUND)
    message(STATUS "Compiling with Curses")
    include_directories(${CURSES_INCLUDE_DIR})
    add_library(Console console.cpp ${CURSES_LIBRARY})
  endif()
else()
  add_library(Console console.cpp)
endif()

add_library(ChatClient chat_client.cpp)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(yeg-chat-client main.cpp)

target_link_libraries(yeg-chat-client Console ChatClient ${Boost_LIBRARIES} ${CURSES_LIBRARY} ${CMAKE_THREAD_LIBS_INIT}) 

