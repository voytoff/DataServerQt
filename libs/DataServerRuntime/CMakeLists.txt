cmake_minimum_required(VERSION 3.20)
project(DataServerRuntime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(DataServerRuntime SHARED
  runtimesystem.h
  formulabuilder.h formulabuilder.cpp
  systembuilder.h systembuilder.cpp
)

target_include_directories(DataServerRuntime
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DataServerRuntime
  PUBLIC
    Configuration
    Formulas
    SignalStorage
    HardwareFactory
    LiveStorage
    SignalProcessor
    Engine
)
