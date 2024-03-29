cmake_minimum_required(VERSION 3.8)
project(SiKRadio)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost COMPONENTS program_options log REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

set(COMMON_SOURCES
        src/Receiver/AudioBuffer.cpp
        src/Utility/Misc.cpp
        src/Utility/Network/Address.cpp
        src/Utility/Network/UDPSocket.cpp
        src/Utility/Reactor/DescriptorResource.cpp
        src/Utility/Reactor/Event.cpp
        src/Utility/Reactor/EventListener.cpp
        src/Utility/Reactor/StreamResource.cpp
        src/Utility/Reactor/Reactor.cpp
        src/Utility/Reactor/Timer.cpp)
link_libraries(-lpthread ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_LOG_LIBRARY} ${Boost_LIBRARIES})
add_definitions(-DBOOST_ALL_DYN_LINK)

set(TRANSMITTER_SOURCES
        src/Transmitter/AudioFIFOBuffer.cpp
        src/Transmitter/ControlComponent.cpp
        src/Transmitter/Main.cpp
        src/Transmitter/SpellCasterComponent.cpp)
add_executable(sikradio-sender ${COMMON_SOURCES} ${TRANSMITTER_SOURCES})
target_include_directories(sikradio-sender PUBLIC src/Utility)

set(RECEIVER_SOURCES
        src/Receiver/LookupComponent.cpp
        src/Receiver/Main.cpp
        src/Receiver/PlayerComponent.cpp)
add_executable(sikradio-receiver ${COMMON_SOURCES} ${RECEIVER_SOURCES})
target_include_directories(sikradio-receiver PUBLIC src/Utility)
