cmake_minimum_required(VERSION 3.23)
project(Logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Logger SHARED
  loglevel.h
  logger.cpp logger.h
  ilogger.h
)

target_include_directories(Logger
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
