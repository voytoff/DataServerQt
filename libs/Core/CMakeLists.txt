cmake_minimum_required(VERSION 3.23)
project(Core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Core SHARED
    iclock.h
    systemclock.h systemclock.cpp
    schedulerclock.h schedulerclock.cpp
    ischedulerclock.h
)

target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(Core PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
