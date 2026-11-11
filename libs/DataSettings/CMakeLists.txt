cmake_minimum_required(VERSION 3.23)

project(datasettings VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS          Core Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql)


add_library(datasettings SHARED
  datasettings_export.h
  applicationsettings.h
  applicationsettingsloader.h applicationsettingsloader.cpp
  databasesettings.h
  databaseconnection.h databaseconnection.cpp
  settingssection.h
  windowsettings.h windowsettings.cpp
  colorschemesettings.h colorschemesettings.cpp
  tablesettings.h
)

target_compile_definitions(datasettings
  PRIVATE
    DATASETTINGS_LIBRARY
)

target_include_directories(datasettings
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(datasettings
  PUBLIC
    Configuration
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)
