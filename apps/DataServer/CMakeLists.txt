cmake_minimum_required(VERSION 3.23)

project(DataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS      Core Widgets Network Sql Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network Sql Svg)

add_library(DataServerCore SHARED
    dataserver.h dataserver.cpp
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
        Logger
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
)

set(SERVER_RESOURCE_FILES res.qrc)
qt6_add_resources(SERVER_PROJECT_RESOURCES ${SERVER_RESOURCE_FILES})
set(APP_ICON_RESOURCE_WINDOWS "appicon.rc")

add_executable(dataserver
    ${SERVER_PROJECT_RESOURCES}
    ${APP_ICON_RESOURCE_WINDOWS}
    main.cpp
    guilogger.h guilogger.cpp
    mainwindow.h mainwindow.cpp
    servercontroller.h servercontroller.cpp
    res.qrc
)

target_link_libraries(dataserver
    PRIVATE
        Configuration
        Formulas
        SignalStorage
        Engine
        HardwareFactory
        UdpSender
        DataServerRuntime
        DataServerCore
        DataArchive
    PRIVATE
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Widgets
      Qt${QT_VERSION_MAJOR}::Network
      Qt${QT_VERSION_MAJOR}::Sql
)

target_include_directories(dataserver
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/qds
)

