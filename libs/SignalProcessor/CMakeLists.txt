cmake_minimum_required(VERSION 3.20)
project(SignalProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(SignalProcessor SHARED
    icalculationprocessor.h
    calculationstep.h
    emptysignalprocessor.cpp emptysignalprocessor.h
    calculationplan.h calculationplan.cpp
    calculationcompiler.h calculationcompiler.cpp


    formulacontextbuilder.h formulacontextbuilder.cpp
    calculationprocessor.h calculationprocessor.cpp
    failingprocessor.h
)

target_include_directories(SignalProcessor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(SignalProcessor
    PUBLIC
        SignalStorage
        Formulas
        Core
)