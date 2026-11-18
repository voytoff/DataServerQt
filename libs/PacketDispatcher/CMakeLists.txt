cmake_minimum_required(VERSION 3.23)
project(packetdispatcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(packetdispatcher SHARED
  packetdispatcher.cpp packetdispatcher.h
)

target_include_directories(packetdispatcher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(packetdispatcher
    PUBLIC
        dataprotocol
    #PRIVATE
        subscriptions
        publisher
        udpsender
        logger
)
