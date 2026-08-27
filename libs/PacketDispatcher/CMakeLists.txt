cmake_minimum_required(VERSION 3.23)
project(PacketDispatcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(PacketDispatcher SHARED
  packetdispatcher.cpp packetdispatcher.h
)

target_include_directories(PacketDispatcher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PacketDispatcher
    PUBLIC
        DataProtocol
    #PRIVATE
        Subscriptions
        Publisher
        UdpSender
)
