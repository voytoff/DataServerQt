cmake_minimum_required(VERSION 3.23)
project(DataWidgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS          Core Widgets Sql Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql Svg)


add_library(DataWidgets SHARED
  datawidgets_export.h
  basetableview.h basetableview.cpp
  configurationselector.h configurationselector.cpp
  themedsvgiconengine.h themedsvgiconengine.cpp
  themedsvgicon.h
  paddeditemdelegate.h paddeditemdelegate.cpp
  colorscheme.h colorscheme.cpp
  applicationsettingsdialog.h applicationsettingsdialog.cpp applicationsettingsdialog.ui
  res.qrc
)

target_include_directories(DataWidgets
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DataWidgets
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Svg
    DataSettings
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