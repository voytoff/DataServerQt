cmake_minimum_required(VERSION 3.20)
project(DataProtocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(DataProtocol SHARED
  packetwriter.h packetwriter.cpp
  packetreader.h packetreader.cpp
  protocol/packetheader.h
  protocol/packettype.h
  protocol/subscribe.h
  protocol/livedata.h
  endpoint.h isender.h
  protocol/subscriptionpackets.h
  protocol/errorpackets.h
  protocol/publishheader.h
  endpoint.cpp
)
target_include_directories(DataProtocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../datatypes
        #${CMAKE_SOURCE_DIR}/libs/datatypes
)

target_link_libraries(DataProtocol PRIVATE
    #Qt${QT_VERSION_MAJOR}::Core
)
