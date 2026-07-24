cmake_minimum_required(VERSION 3.20)

project(DataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/configuration)
add_subdirectory(libs/livestorage)
add_subdirectory(libs/publisher)
add_subdirectory(libs/subscriptions)
add_subdirectory(libs/dataprotocol)
add_subdirectory(libs/livescheduler)
add_subdirectory(libs/udpsender)
add_subdirectory(libs/udpserver)
add_subdirectory(libs/datasource)
add_subdirectory(libs/core)
add_subdirectory(libs/engine)
add_subdirectory(libs/hardware)
add_subdirectory(libs/hardwarefactory)
add_subdirectory(libs/moduleloader)
add_subdirectory(libs/packetdispatcher)

enable_testing()
add_subdirectory(tests)

add_subdirectory(apps/dataserver)
