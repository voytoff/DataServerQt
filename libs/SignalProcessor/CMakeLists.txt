cmake_minimum_required(VERSION 3.20)
project(SignalProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(SignalProcessor SHARED
  signalprocessor.cpp
  signalprocessor.h
)

target_include_directories(SignalProcessor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
