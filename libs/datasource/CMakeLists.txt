cmake_minimum_required(VERSION 3.20)
project(DataSource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(DataSource SHARED

  idatasource.h
  imoduledatasink.h
  generatordatasource.h generatordatasource.cpp
  imodulegenerator.h
  countergenerator.h countergenerator.cpp
  periodicdatasourcerunner.h periodicdatasourcerunner.cpp
)

target_include_directories(DataSource
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../configuration
)

target_link_libraries(DataSource PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Configuration
)

# target_compile_definitions(DataSource PRIVATE CONFIGURATION_LIBRARY)