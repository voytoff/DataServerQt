cmake_minimum_required(VERSION 3.23)
project(dataexport VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS           Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core)

add_library(dataexport SHARED
  dataexport_global.h
  archivetableexportsettings.h
  archivetablewriter.h
  archivetableexporter.h archivetableexporter.cpp
)

target_compile_definitions(dataexport
  PRIVATE DATAEXPORT_LIBRARY
)

target_include_directories(dataexport
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dataexport
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    dataarchive
    datawidgets
)
