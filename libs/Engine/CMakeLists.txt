cmake_minimum_required(VERSION 3.20)
project(Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Engine SHARED
  dataengine.h dataengine.cpp
)

target_include_directories(Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../core
        ../datasource
        ../livescheduler
)

target_link_libraries(Engine PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Core
    DataSource
    LiveScheduler
)
