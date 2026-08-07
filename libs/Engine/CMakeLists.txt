cmake_minimum_required(VERSION 3.20)
project(Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Engine SHARED
  dataengine.h dataengine.cpp
)

target_include_directories(Engine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(Engine
    PUBLIC
        Core
        DataSource
        SignalProcessor
        DataArchive
        Publisher
)
