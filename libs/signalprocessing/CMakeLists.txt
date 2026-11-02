cmake_minimum_required(VERSION 3.23)
project(signalprocessing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(signalprocessing SHARED
  signalresampler.cpp
  signalresampler.h
)

target_compile_features(signalprocessing PUBLIC cxx_std_23)

target_include_directories(signalprocessing
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(signalprocessing
  PUBLIC
    DataArchive
)