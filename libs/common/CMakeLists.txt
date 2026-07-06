cmake_minimum_required(VERSION 3.21)

project(Common LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Common SHARED
  Common_global.h
  common.cpp
  common.h
  datatypes.h
  command.h command.cpp
  memorystorage.h memorystorage.cpp
  tag.h tag.cpp
  operation.h operation.cpp
  memory.h memory.cpp
  dataengine.h dataengine.cpp
  linearoperation.h linearoperation.cpp
  sumoperation.h sumoperation.cpp
)

target_link_libraries(Common PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(Common PRIVATE COMMON_LIBRARY)
