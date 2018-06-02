cmake_minimum_required(VERSION 3.10)
project(Reactor)

set(CMAKE_CXX_STANDARD 17)

add_executable(Reactor Main.cpp Reactor.cpp Reactor.hpp)
target_link_libraries(Reactor -lpthread)