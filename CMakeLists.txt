cmake_minimum_required(VERSION 3.20)

project(QtDataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_subdirectory(tests)
add_subdirectory(libs/configuration)
add_subdirectory(libs/livestorage)
add_subdirectory(libs/publisher)
add_subdirectory(libs/subscriptions)
add_subdirectory(libs/dataprotocol)