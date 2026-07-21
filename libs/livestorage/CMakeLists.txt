cmake_minimum_required(VERSION 3.20)
project(LiveStorage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(LiveStorage SHARED
  livestorage.cpp
  livestorage.h
)

target_include_directories(LiveStorage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../configuration
        ../datasource
)

target_link_libraries(LiveStorage PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Configuration
    DataSource
)

target_compile_definitions(LiveStorage PRIVATE CONFIGURATION_LIBRARY)