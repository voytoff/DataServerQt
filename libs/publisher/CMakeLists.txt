cmake_minimum_required(VERSION 3.20)
project(Publisher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Publisher SHARED
  publisher.cpp publisher.h

)

target_include_directories(Publisher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../subscriptions
        ../dataprotocol
        ../livestorage
)

target_link_libraries(Publisher PRIVATE
    PUBLIC
        LiveStorage
        Subscriptions
        DataProtocol

    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
)