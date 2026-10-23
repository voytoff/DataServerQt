cmake_minimum_required(VERSION 3.23)
project(datamonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS           Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Widgets Sql)

qt_add_executable(datamonitor
    main.cpp
    mainwindow.h mainwindow.cpp
    model/signalselectionmodel.h model/signalselectionmodel.cpp
)

target_include_directories(datamonitor
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
)

target_link_libraries(datamonitor
  PRIVATE
    Configuration
    DataClient
    DataSettings
    DataWidgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)