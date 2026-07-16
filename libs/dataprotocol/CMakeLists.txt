cmake_minimum_required(VERSION 3.20)
project(DataProtocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(DataProtocol SHARED
  packetwriter.h packetwriter.cpp
  packetreader.h packetreader.cpp
  protocol/packetheader.h protocol/packetheader.cpp
  protocol/packettype.h protocol/packettype.cpp
  protocol/subscribe.h protocol/subscribe.cpp
  protocol/livedata.h
  endpoint.h isender.h


  protocol/subscriptionpackets.h protocol/subscriptionpackets.cpp
)
target_include_directories(DataProtocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        #../subscriptions
)

target_link_libraries(DataProtocol PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

target_compile_definitions(DataProtocol PRIVATE CONFIGURATION_LIBRARY)