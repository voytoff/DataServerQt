cmake_minimum_required(VERSION 3.23)
project(modulevalidators LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(modulevalidators SHARED
  modulevalidators.h
  modulevalidators.cpp
)

target_include_directories(modulevalidators
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(modulevalidators
  PUBLIC
    configurationvalidation
  PRIVATE
    DataSource
)