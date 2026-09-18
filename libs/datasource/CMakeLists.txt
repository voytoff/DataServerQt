cmake_minimum_required(VERSION 3.23)
project(DataSource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(DataSource SHARED
  iactivedatasource.h
  imoduledatasink.h
  imodulegenerator.h
  idatabuffer.h
  idatasource.h
  idatablocksink.h
  failingdatasource.h
  datastreamanchor.h
  idatastreameventsink.h
  datablock.h
  moduleconfiguration.h
  generatordatasource.h generatordatasource.cpp
  countergenerator.h countergenerator.cpp
  datasourcemanager.h datasourcemanager.cpp
  fakeactivedatasource.h fakeactivedatasource.cpp
  moduledatasource.h moduledatasource.cpp
  fakedatasource.h fakedatasource.cpp
  testdatasource.h testdatasource.cpp
  datasourcefactory.h datasourcefactory.cpp
  failoncedatasource.h failoncedatasource.cpp
  lcarddatasource.h lcarddatasource.cpp
  ltr11configurationbuilder.h ltr11configurationbuilder.cpp
  fakedatablocksink.h fakedatablocksink.cpp
)

target_include_directories(DataSource
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(DataSource
  PUBLIC
    Core
    Configuration
    Hardware
    SignalStorage
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)
