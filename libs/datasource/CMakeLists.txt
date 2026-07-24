cmake_minimum_required(VERSION 3.20)
project(DataSource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(DataSource SHARED
  idatasource.h
  imoduledatasink.h
  imodulegenerator.h
  generatordatasource.h generatordatasource.cpp
  countergenerator.h countergenerator.cpp
  periodicdatasourcerunner.h periodicdatasourcerunner.cpp
  datasourcemanager.h datasourcemanager.cpp
  fakedatasource.h fakedatasource.cpp
  moduledatasource.h moduledatasource.cpp
)

target_include_directories(DataSource
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../core
        ../configuration
        ../hardware
)

target_link_libraries(DataSource PRIVATE
  PUBLIC
    Core
    Configuration
    Hardware

  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
