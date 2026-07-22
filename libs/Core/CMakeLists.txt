cmake_minimum_required(VERSION 3.20)
project(Core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Core SHARED
    iclock.h
    systemclock.h systemclock.cpp
    fakeclock.h fakeclock.cpp
)

target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Core PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
