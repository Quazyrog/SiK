cmake_minimum_required(VERSION 3.9)
project(TelnetTask)

set(CMAKE_CXX_STANDARD 17)

add_executable(TelnetTask
        src/Main.cpp
        src/TCPSocket.cpp src/TCPSocket.hpp
        src/Util.hpp
        src/Telnet.cpp src/Telnet.hpp
        src/RemoteTerminal.cpp src/RemoteTerminal.hpp src/MenuApplication.cpp src/MenuApplication.hpp)
