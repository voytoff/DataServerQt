cmake_minimum_required(VERSION 3.20)
project(LiveScheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(LiveScheduler SHARED
  livescheduler.cpp
  livescheduler.h
)

target_include_directories(LiveScheduler
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../subscriptions
        ../publisher
        ../livestorage
        ../dataprotocol
)

target_link_libraries(LiveScheduler PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Subscriptions
    Publisher
    LiveStorage
    DataProtocol
)

target_compile_definitions(LiveScheduler PRIVATE CONFIGURATION_LIBRARY)