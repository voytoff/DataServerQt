cmake_minimum_required(VERSION 3.20)
project(UdpSender LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_library(UdpSender SHARED
  udpsender.cpp udpsender.h
)

target_include_directories(Subscriptions
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        ../livescheduler
        ../dataprotocol
)

target_link_libraries(UdpSender PRIVATE
  PUBLIC
    LiveScheduler
    DataProtocol
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)
