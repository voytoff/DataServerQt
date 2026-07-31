cmake_minimum_required(VERSION 3.20)
project(Configuration LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Configuration SHARED
    moduletype.h
    taginfo.cpp taginfo.h
    moduleinfo.cpp moduleinfo.h
    crateinfo.cpp crateinfo.h
    systemconfiguration.cpp systemconfiguration.h
    signaldefinition.h signaldefinition.cpp
)

target_include_directories(Configuration
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)
