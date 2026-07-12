cmake_minimum_required(VERSION 3.20)
project(test_common LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Network)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_common
    main.cpp
    tst_common.h tst_common.cpp
    tst_udpsender.h tst_udpsender.cpp
)

target_include_directories(test_common PRIVATE
    ../libs/configuration
    ../libs/configuration
    ../libs/publisher
    ../libs/subscriptions
    ../libs/dataprotocol
    ../libs/livescheduler
    ../libs/udpsender
)

target_link_libraries(test_common PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Network
    Configuration
    LiveStorage
    Subscriptions
    DataProtocol
    Publisher
    LiveScheduler
    UdpSender
)

add_test(NAME test_common COMMAND test_common)
