cmake_minimum_required(VERSION 3.23)
project(dataserverruntime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dataserverruntime SHARED
  runtimesystem.h
  systembuilder.h systembuilder.cpp
)

target_include_directories(dataserverruntime
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dataserverruntime
  PUBLIC
    configuration
    formulas
    signalstorage
    signalprocessor
    engine
    configurationvalidation
)
