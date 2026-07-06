cmake_minimum_required(VERSION 3.20)

project(QtDataServerApp LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(DIR1 "../../src/common")
include_directories(${DIR1})
add_subdirectory(${DIR1} Common)

set(DIR2 "../../src/server")
include_directories(${DIR2})
add_subdirectory(${DIR2} Server)

add_executable(QtDataServerApp
    main.cpp
)

target_link_libraries(QtDataServerApp Qt6::Core Qt6::Network Common Server)