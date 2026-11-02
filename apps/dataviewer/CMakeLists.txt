cmake_minimum_required(VERSION 3.23)
project(dataviewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS           Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Widgets)

set(DATAVIEWER_RESOURCE_FILES res.qrc)
qt6_add_resources(DATAVIEWER_PROJECT_RESOURCES ${DATAVIEWER_RESOURCE_FILES})
set(APP_ICON_RESOURCE_WINDOWS "appicon.rc")

qt_add_executable(dataviewer
  ${DATAVIEWER_PROJECT_RESOURCES}
  ${APP_ICON_RESOURCE_WINDOWS}
  main.cpp
)

target_include_directories(dataviewer
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/datatypes
)

target_link_libraries(dataviewer
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Configuration
    DataSettings
    DataWidgets
    signalprocessing
)