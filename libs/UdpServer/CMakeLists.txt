cmake_minimum_required(VERSION 3.20)
project(UdpServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_library(UdpServer
  udpserver.cpp
  udpserver.h
)

target_include_directories(Subscriptions
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../dataprotocol
        ../subscriptions
        ../livescheduler
        ../configuration
        ../datasource
)

target_link_libraries(UdpServer PRIVATE
  PUBLIC
    DataProtocol
    Subscriptions
    LiveScheduler
    Configuration

  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)

target_compile_options(UdpServer PRIVATE -Wno-nonportable-include-path)