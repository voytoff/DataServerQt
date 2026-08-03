cmake_minimum_required(VERSION 3.20)
project(SignalStorage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(SignalStorage SHARED
  signalarea.h signalarea.cpp
  signalstorage.cpp signalstorage.h
  signalmemorylayout.h signalmemorylayout.cpp
  buffermanager.h buffermanager.cpp
  frame.h frame.cpp
  rawmemory.h rawmemory.cpp
)

target_include_directories(SignalStorage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(SignalStorage
    PUBLIC
        Configuration
)