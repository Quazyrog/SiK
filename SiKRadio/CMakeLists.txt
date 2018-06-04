cmake_minimum_required(VERSION 3.9)
project(SiKRadio)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_SOURCES
        src/Utility/Config.cpp
        src/Utility/Reactor/DescriptorResource.cpp
        src/Utility/Reactor/Event.cpp
        src/Utility/Reactor/EventListener.cpp
        src/Utility/Reactor/InputStreamResource.cpp
        src/Utility/Reactor/Reactor.cpp
        src/Utility/Reactor/Timer.cpp)
link_libraries(-lpthread)

set(TRANSMITTER_SOURCES
        src/transmitter/Main.cpp)
add_executable(nadajnik ${COMMON_SOURCES} ${TRANSMITTER_SOURCES})
target_include_directories(nadajnik PUBLIC src/Utility)

set(RECEIVER_SOURCES
        src/Receiver/LookupComponent.cpp
        src/Receiver/Main.cpp)
add_executable(odbiornik ${COMMON_SOURCES} ${RECEIVER_SOURCES})
target_include_directories(odbiornik PUBLIC src/Utility)
