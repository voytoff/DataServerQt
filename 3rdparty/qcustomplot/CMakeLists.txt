cmake_minimum_required(VERSION 3.23)
project(QCustomPlot VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    PrintSupport
)

add_library(QCustomPlot STATIC
    qcustomplot.h
    qcustomplot.cpp
)

target_include_directories(QCustomPlot
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(QCustomPlot
    PUBLIC
        Qt6::Widgets
        Qt6::PrintSupport
)

if(MINGW)
    target_compile_options(QCustomPlot PRIVATE
        -Wa,-mbig-obj
    )
endif()