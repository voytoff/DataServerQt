cmake_minimum_required(VERSION 3.23)
project(Publisher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Publisher SHARED
  publisher.cpp publisher.h
  iframepublisher.h
  nullframepublisher.h nullframepublisher.cpp
  testpublisher.h testpublisher.cpp
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
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
    PUBLIC
        Configuration
        DataSource
    #LiveStorage
        Subscriptions
        DataProtocol
)