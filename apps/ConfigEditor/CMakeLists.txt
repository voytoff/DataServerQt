cmake_minimum_required(VERSION 3.23)
project(configeditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)

qt_add_executable(configeditor
    main.cpp

    mainwindow.h mainwindow.cpp
    configurationeditor.h configurationeditor.cpp
    configurationtreemodel.h configurationtreemodel.cpp
    treeitem.h treeitem.cpp
    treeitemdata.h

)

target_include_directories(configeditor
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/datatypes
)

target_link_libraries(configeditor
    PRIVATE
        Configuration
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
)