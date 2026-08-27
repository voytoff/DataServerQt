cmake_minimum_required(VERSION 3.23)
project(DataSource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(DataSource SHARED
  iactivedatasource.h
  imoduledatasink.h
  imodulegenerator.h
  generatordatasource.h generatordatasource.cpp
  countergenerator.h countergenerator.cpp
  datasourcemanager.h datasourcemanager.cpp
  fakeactivedatasource.h fakeactivedatasource.cpp
  moduledatasource.h moduledatasource.cpp
  idatabuffer.h
  idatasource.h
  fakedatasource.h fakedatasource.cpp
  testdatasource.h testdatasource.cpp
  failingdatasource.h
  datasourcefactory.h datasourcefactory.cpp
  failoncedatasource.h failoncedatasource.cpp
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
)
