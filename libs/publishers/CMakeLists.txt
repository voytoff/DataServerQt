cmake_minimum_required(VERSION 3.20)
project(Publishers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Publishers SHARED
  publisher.cpp publisher.h

)

target_include_directories(Publishers
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../subscriptions
        ../dataprotocol
        ../livestorage
)

target_link_libraries(Publishers PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    LiveStorage
    Subscriptions
    DataProtocol
)

target_compile_definitions(Publishers PRIVATE CONFIGURATION_LIBRARY)