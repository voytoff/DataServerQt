cmake_minimum_required(VERSION 3.23)
project(DataProtocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(DataProtocol SHARED
  packetwriter.h packetwriter.cpp
  packetreader.h packetreader.cpp
  protocol/packetheader.h
  protocol/packettype.h
  protocol/livedata.h
  endpoint.h isender.h
  protocol/subscriptionpackets.h
  protocol/errorpackets.h
  protocol/publishheader.h
  endpoint.cpp
  testpublishersender.h testpublishersender.cpp
)
target_include_directories(DataProtocol
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)
