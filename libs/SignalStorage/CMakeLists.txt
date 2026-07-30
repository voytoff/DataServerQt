cmake_minimum_required(VERSION 3.20)
project(SignalStorage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(SignalStorage SHARED
  signalstorage.cpp signalstorage.h
  signaldefinition.h
  signalprocessor.h
  signalframe.h
)

target_include_directories(SignalStorage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)
