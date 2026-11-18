cmake_minimum_required(VERSION 3.23)
project(subscriptions LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(subscriptions SHARED
  subscription.h
  subscriptionmanager.h subscriptionmanager.cpp
)

target_include_directories(subscriptions
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(subscriptions
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
    PUBLIC
        dataprotocol
)