cmake_minimum_required(VERSION 3.20)
project(Hardware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Hardware SHARED
    ihardwaremodule.h
    lcardmodule.h lcardmodule.cpp
    fakelcardmodule.h fakelcardmodule.cpp
)

target_include_directories(Hardware
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(Hardware
    PUBLIC
        Configuration
)
