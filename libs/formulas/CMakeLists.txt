cmake_minimum_required(VERSION 3.20)
project(Formulas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Formulas SHARED
  iformula.h
  formularepository.h formularepository.cpp
  formulacopy.cpp formulacopy.h
  formulaadd.h formulaadd.cpp
  formulacontext.h
)

target_include_directories(Formulas
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(Formulas
    PUBLIC
        Configuration
        SignalStorage
)