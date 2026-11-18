cmake_minimum_required(VERSION 3.23)
project(publisher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(publisher SHARED
  publisher.cpp publisher.h
  iframepublisher.h
)

target_include_directories(publisher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../subscriptions
        ../dataprotocol
        ../livestorage
)

target_link_libraries(publisher PRIVATE
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
    PUBLIC
        configuration
        datasource
        subscriptions
        dataprotocol
)