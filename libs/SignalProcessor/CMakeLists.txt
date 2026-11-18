cmake_minimum_required(VERSION 3.23)
project(signalprocessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(signalprocessor SHARED
    calculationstep.h
    calculationplan.h calculationplan.cpp
    calculationcompiler.h calculationcompiler.cpp
    signalprocessor.h signalprocessor.cpp
)

target_include_directories(signalprocessor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(signalprocessor
    PUBLIC
        signalstorage
        formulas
        core
)