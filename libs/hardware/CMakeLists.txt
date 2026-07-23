cmake_minimum_required(VERSION 3.20)
project(Hardware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Hardware SHARED
    imoduledevice.h
  lcardmodule.h lcardmodule.cpp
  fakelcardmodule.h fakelcardmodule.cpp
  moduledatasource.h moduledatasource.cpp
)

target_include_directories(Hardware
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datasource
        ../livescheduler
        ../configuration
    )

target_link_libraries(Hardware PRIVATE
    PUBLIC
        Core
        DataSource
        LiveScheduler
        Configuration

    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
)
