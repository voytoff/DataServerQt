cmake_minimum_required(VERSION 3.20)
project(UdpServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_library(UdpServer
    udpserver.h udpserver.cpp
)

target_include_directories(Subscriptions
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../dataprotocol
        ../subscriptions
        ../livescheduler
        ../configuration
)

target_link_libraries(UdpServer PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Network
  DataProtocol
  Subscriptions
  LiveScheduler
  Configuration
)

target_compile_definitions(UdpServer PRIVATE CONFIGURATION_LIBRARY)