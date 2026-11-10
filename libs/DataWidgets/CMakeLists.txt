cmake_minimum_required(VERSION 3.23)
project(datawidgets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS          Core Widgets Sql Svg Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql Svg Charts)

add_library(datawidgets SHARED
  datawidgets_export.h
  themedsvgicon.h
  basetableview.h basetableview.cpp
  configurationselector.h configurationselector.cpp
  themedsvgiconengine.h themedsvgiconengine.cpp
  paddeditemdelegate.h paddeditemdelegate.cpp
  colorscheme.h colorscheme.cpp
  applicationsettingsdialog.h applicationsettingsdialog.cpp applicationsettingsdialog.ui
  rowheightdelegate.h rowheightdelegate.cpp
  timeaxis.h timeaxis.cpp
  timechartview.h timechartview.cpp
  timerangecontroller.h timerangecontroller.cpp
  archivechartseries.h archivechartseries.cpp
  archivechartcontroller.h archivechartcontroller.cpp
  archivechartworker.h archivechartworker.cpp
  archivechartview.h archivechartview.cpp
  chartpalette.h chartpalette.cpp
  archivetablemodel.h archivetablemodel.cpp
)

target_include_directories(datawidgets
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(datawidgets
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Charts
    DataSettings
    DataArchive
)

target_compile_definitions(datawidgets
  PRIVATE
    DATAWIDGETS_LIBRARY
)

target_include_directories(datawidgets
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/datatypes
)