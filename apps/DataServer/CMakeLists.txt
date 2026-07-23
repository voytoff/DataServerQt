cmake_minimum_required(VERSION 3.20)

project(DataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_executable(dataserver
    main.cpp
    dataserver.cpp
    dataserver.h
)

target_include_directories(dataserver PRIVATE
    ../../libs/core
    ../../libs/configuration
    ../../libs/configuration
    ../../libs/publisher
    ../../libs/subscriptions
    ../../libs/dataprotocol
    ../../libs/livescheduler
    ../../libs/udpsender
    ../../libs/udpserver
    ../../libs/datasource
    ../../libs/engine
    ../../libs/hardware
)

target_link_libraries(dataserver PRIVATE
    Qt6::Core
    Qt6::Network
    Core
)