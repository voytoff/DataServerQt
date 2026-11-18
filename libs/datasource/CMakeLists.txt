cmake_minimum_required(VERSION 3.23)
project(datasource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(datasource SHARED
  iactivedatasource.h
  imoduledatasink.h
  imodulegenerator.h
  idatabuffer.h
  idatasource.h
  idatablocksink.h
  datastreamanchor.h
  idatastreameventsink.h
  datablock.h
  moduleconfiguration.h
  datasourcemanager.h datasourcemanager.cpp
  moduledatasource.h moduledatasource.cpp
  datasourcefactory.h datasourcefactory.cpp
  lcarddatasource.h lcarddatasource.cpp
  ltr11configurationbuilder.h ltr11configurationbuilder.cpp
  datastreamevent.h
  datastreamtime.h datastreamtime.cpp
  idatastreamsource.h
  datastreamsourcefactory.h datastreamsourcefactory.cpp
  datastreamsourcemanager.h datastreamsourcemanager.cpp
  datastreamsourcestatistics.h
  datastreamsourcestate.h
)

target_include_directories(datasource
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(datasource
  PUBLIC
    core
    configuration
    hardware
    signalstorage
  PRIVATE
    logger
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)
