cmake_minimum_required(VERSION 3.23)
project(DataWidgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS      Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)


add_library(DataWidgets SHARED
  datawidgets_export.h
  basetableview.h basetableview.cpp
  configurationselector.h configurationselector.cpp
)

target_include_directories(DataWidgets
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DataWidgets
  PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

target_compile_definitions(DataWidgets
  PRIVATE
    DATAWIDGETS_LIBRARY
)

target_include_directories(DataWidgets
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/datatypes
)