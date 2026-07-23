cmake_minimum_required(VERSION 3.20)
project(unit_tests LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Network)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(unit_tests
    main.cpp
    tst_common.h tst_common.cpp
    tst_udpsender.h tst_udpsender.cpp
    tst_subscriptions.h tst_subscriptions.cpp
    tst_livestorage.h tst_livestorage.cpp
    tst_livescheduler.h tst_livescheduler.cpp
    tst_udpserver.h tst_udpserver.cpp
    test_dataprotocol.h test_dataprotocol.cpp
    testsrv.h testsrv.cpp
    tst_publisher.h tst_publisher.cpp
    tst_datasource.h tst_datasource.cpp
    tst_core.h tst_core.cpp
    tst_engine.h tst_engine.cpp
    tst_hardware.h tst_hardware.cpp
)

target_link_libraries(unit_tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Network
    Core
    Configuration
    LiveStorage
    Subscriptions
    DataProtocol
    Publisher
    LiveScheduler
    UdpSender
    UdpServer
    DataSource
    Engine
    Hardware
)

add_test(NAME unit_tests COMMAND unit_tests)
