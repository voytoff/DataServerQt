cmake_minimum_required(VERSION 3.20)
project(Configuration LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Configuration SHARED
    taginfo.cpp taginfo.h
    moduleinfo.cpp moduleinfo.h
    crateinfo.cpp crateinfo.h
    systemconfiguration.cpp systemconfiguration.h
)

target_include_directories(Configuration
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
)

target_link_libraries(Configuration PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

target_compile_definitions(Configuration PRIVATE CONFIGURATION_LIBRARY)