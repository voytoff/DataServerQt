cmake_minimum_required(VERSION 3.20)
project(Subscriptions LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(Subscriptions SHARED
  subscription.cpp subscription.h
  subscriptionmanager.h subscriptionmanager.cpp
)

target_include_directories(Subscriptions
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Subscriptions PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    DataProtocol
)
