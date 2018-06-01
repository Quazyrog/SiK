cmake_minimum_required(VERSION 3.9)
project(SiKRadio)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_SOURCES
        src/commons/Config.hpp)
set(TRANSMITTER_SOURCES
        src/transmitter/Main.cpp)
set(RECEIVER_SOURCES
        src/receiver/Main.cpp)

add_executable(nadajnik ${COMMON_SOURCES} ${TRANSMITTER_SOURCES})
target_include_directories(nadajnik PUBLIC src/commons src/transmitter)

add_executable(odbiornik ${COMMON_SOURCES} ${RECEIVER_SOURCES})
target_include_directories(odbiornik PUBLIC src/commons src/receiver)
