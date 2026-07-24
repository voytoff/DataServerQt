cmake_minimum_required(VERSION 3.20)
project(HardwareFactory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(HardwareFactory SHARED
  ihardwaremodulefactory.h
  hardwaremodulefactory.h hardwaremodulefactory.cpp
)

target_include_directories(HardwareFactory
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(HardwareFactory
    PUBLIC
        Configuration
        Hardware
)
