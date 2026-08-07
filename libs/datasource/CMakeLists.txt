cmake_minimum_required(VERSION 3.20)
project(DataSource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(DataSource SHARED
  iactivedatasource.h
  imoduledatasink.h
  imodulegenerator.h
  generatordatasource.h generatordatasource.cpp
  countergenerator.h countergenerator.cpp
  periodicdatasourcerunner.h periodicdatasourcerunner.cpp
  datasourcemanager.h datasourcemanager.cpp
  fakeactivedatasource.h fakeactivedatasource.cpp
  moduledatasource.h moduledatasource.cpp
  modulebuffer.h modulebuffer.cpp
  idatabuffer.h
  idatasource.h
  fakedatasource.h fakedatasource.cpp
  testdatasource.h testdatasource.cpp
  failingdatasource.h
)

target_include_directories(DataSource
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
)

target_link_libraries(DataSource
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core

  PUBLIC
    Core
    Configuration
    Hardware
    SignalStorage
)
