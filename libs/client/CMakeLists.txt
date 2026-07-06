cmake_minimum_required(VERSION 3.20)

project(Client LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Client SHARED
  Client_global.h
  client.cpp
  client.h
)

target_link_libraries(Client PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(Client PRIVATE CLIENT_LIBRARY)
