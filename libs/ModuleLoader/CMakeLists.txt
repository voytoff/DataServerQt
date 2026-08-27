cmake_minimum_required(VERSION 3.23)
project(ModuleLoader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ModuleLoader SHARED
  moduleloader.cpp moduleloader.h
)

target_include_directories(ModuleLoader
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ModuleLoader PRIVATE
    Configuration
    Hardware
    HardwareFactory
    DataSource
    Core
)
