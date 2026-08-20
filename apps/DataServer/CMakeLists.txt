cmake_minimum_required(VERSION 3.20)

project(DataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_library(DataServerCore SHARED
    dataserver.h
    dataserver.cpp
)

target_include_directories(DataServerCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DataServerCore
    PUBLIC
        PacketDispatcher
        DataServerRuntime
        UdpServer
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
)

add_executable(dataserver
    main.cpp
)

target_link_libraries(dataserver
    PRIVATE
        Configuration
        Formulas
        SignalStorage
        Engine
        HardwareFactory
        UdpSender
        #UdpServer
        DataServerRuntime
        DataServerCore

    PRIVATE
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Network
)
