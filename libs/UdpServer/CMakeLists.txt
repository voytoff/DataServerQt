cmake_minimum_required(VERSION 3.23)
project(udpserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_library(udpserver SHARED
    udpserver.h
    udpserver.cpp
)

target_include_directories(udpserver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(udpserver
    PUBLIC
        packetdispatcher
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
)

target_compile_options(udpserver
    PRIVATE
        -Wno-nonportable-include-path
)