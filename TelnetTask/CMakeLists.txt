cmake_minimum_required(VERSION 3.9)
project(TelnetTask)

set(CMAKE_CXX_STANDARD 17)

add_executable(TelnetTask
        src/Main.cc
        src/TCPSocket.cc src/TCPSocket.hpp
        src/Util.hpp
        src/Telnet.cc src/Telnet.hpp
        src/RemoteTerminal.cc src/RemoteTerminal.hpp src/MenuApplication.cc src/MenuApplication.hpp)
