cmake_minimum_required(VERSION 3.23)

project(DataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS           Core Widgets Network Sql Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Widgets Network Sql Svg)

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
        configurationvalidation
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
)

get_target_property(
  QT_QMAKE_EXECUTABLE
  Qt6::qmake
  IMPORTED_LOCATION
)

get_filename_component(
  QT_BIN_DIR
  "${QT_QMAKE_EXECUTABLE}"
  DIRECTORY
)

find_program(
  WINDEPLOYQT_EXECUTABLE
 +
 +*NAMES windeployqt
  HINTS "${QT_BIN_DIR}"
  REQUIRED
)

add_custom_command(
  TARGET dataserver
  POST_BUILD

  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:dataserver>
    $<TARGET_FILE_DIR:dataserver>

  COMMAND "${WINDEPLOYQT_EXECUTABLE}"
    --dir "$<TARGET_FILE_DIR:dataserver>"
    "$<TARGET_FILE:dataserver>"

  COMMAND_EXPAND_LISTS
  VERBATIM
)

target_link_libraries(dataserver
    PRIVATE
      Configuration
      Formulas
      SignalStorage
      Engine
      UdpSender
      DataServerRuntime
      DataServerCore
      DataArchive
      DataSettings
      DataWidgets
      modulevalidators
      configurationvalidation
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

