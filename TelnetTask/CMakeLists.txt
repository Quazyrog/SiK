cmake_minimum_required(VERSION 3.9)
project(TelnetTask)

set(CMAKE_CXX_STANDARD 17)

add_executable(TelnetTask Main.cpp TCPSocket.cpp TCPSocket.hpp Util.hpp)