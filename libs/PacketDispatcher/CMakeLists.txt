cmake_minimum_required(VERSION 3.20)
project(PacketDispatcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(PacketDispatcher SHARED
  packetdispatcher.cpp packetdispatcher.h
)

target_include_directories(PacketDispatcher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PacketDispatcher PRIVATE
    DataProtocol
    Subscriptions
    LiveScheduler
    Publisher
    UdpSender
)
