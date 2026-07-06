cmake_minimum_required(VERSION 3.20)
project(QtDataClient VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
)
qt_standard_project_setup()

add_executable(QtDataClient
    main.cpp
    PacketType.h
    client.h client.cpp
)

target_link_libraries(QtDataClient
    Qt6::Core
    Qt6::Network
)