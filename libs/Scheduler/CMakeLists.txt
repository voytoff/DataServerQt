cmake_minimum_required(VERSION 3.20)
project(Scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Scheduler SHARED
  scheduler.cpp scheduler.h
)

target_include_directories(Scheduler
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(Scheduler
    PUBLIC
        Core
        DataSource
        SignalStorage
        SignalProcessor
)