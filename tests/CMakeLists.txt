cmake_minimum_required(VERSION 3.23)
project(unit_tests LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Core Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Network Sql)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(unit_tests
    main.cpp
    db.h
    tst_common.h tst_common.cpp
    tst_udpsender.h tst_udpsender.cpp
    tst_subscriptions.h tst_subscriptions.cpp
    #tst_livestorage.h tst_livestorage.cpp
    #tst_livescheduler.h tst_livescheduler.cpp
    tst_udpserver.h tst_udpserver.cpp
    tst_dataprotocol.h tst_dataprotocol.cpp
    testsrv.h testsrv.cpp
    tst_publisher.h tst_publisher.cpp
    tst_datasource.h tst_datasource.cpp
    tst_core.h tst_core.cpp
    tst_engine.h tst_engine.cpp
    tst_hardware.h tst_hardware.cpp
    tst_packetdispatcher.h tst_packetdispatcher.cpp
    tst_dataarchive.h tst_dataarchive.cpp
    tst_signalstorage.h tst_signalstorage.cpp
    #tst_scheduler.h tst_scheduler.cpp
    tst_signalprocessor.h tst_signalprocessor.cpp
    tst_configuration.h tst_configuration.cpp
    tst_formulas.h tst_formulas.cpp
    tst_dataserver.h tst_dataserver.cpp
    tst_database.h tst_database.cpp
    udp.h
)

target_link_libraries(unit_tests
  PRIVATE
    Core
    Configuration
    DataServerRuntime
    Subscriptions
    DataProtocol
    DataServerCore
    Publisher
    UdpSender
    DataSource
    Engine
    HardwareFactory
    DataArchive
    SignalStorage
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
)

add_test(NAME unit_tests COMMAND unit_tests)
