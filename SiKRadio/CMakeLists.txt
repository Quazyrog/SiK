cmake_minimum_required(VERSION 3.9)
project(SiKRadio)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_SOURCES
        src/Utility/Config.cpp
        src/Utility/Reactor/Timer.cpp
        src/Utility/Reactor/Reactor.cpp)
set(TRANSMITTER_SOURCES
        src/transmitter/Main.cpp src/receiver/Menu/Handler.cpp src/receiver/Menu/Handler.h)
set(RECEIVER_SOURCES
        src/receiver/Main.cpp
        src/receiver/EventHub.cpp
        src/receiver/EventInput.cpp
        src/receiver/Control/Handler.cpp
        src/receiver/Data/Handler.cpp
        src/receiver/Menu/Handler.cpp)

add_executable(nadajnik ${COMMON_SOURCES} ${TRANSMITTER_SOURCES})
target_include_directories(nadajnik PUBLIC src/Utility)

add_executable(odbiornik ${COMMON_SOURCES} ${RECEIVER_SOURCES})
target_include_directories(odbiornik PUBLIC src/Utility)
