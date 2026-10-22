cmake_minimum_required(VERSION 3.23)
project(DataClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS      Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_library(DataClient SHARED
  clientconnection.h clientconnection.cpp
  dataclient_global.h
)

target_compile_definitions(DataClient
  PRIVATE
    DATACLIENT_LIBRARY
)

target_include_directories(DataClient
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
)

target_link_libraries(DataClient
  PRIVATE
    Configuration
    Subscriptions
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
  )