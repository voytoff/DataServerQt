cmake_minimum_required(VERSION 3.23)
project(unit_tests LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

enable_testing()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Test Core Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Network Sql)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(unit_tests
  main.cpp
  qds/db.h
  qds/udp.h
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
  tst_logger.h tst_logger.cpp
  tst_ltr11module.h tst_ltr11module.cpp
  qds/fakedatastreameventsink.h
  #qds/testarchiveframewriter.h
  qds/testpublisher.h
  qds/testarchivewriter.h
  qds/testdatastreamsource.h
  qds/testpublishersender.h
  qds/nullarchivewriter.h
  qds/teststreamingdatasource.h qds/teststreamingdatasource.cpp
  tst_configeditor.h tst_configeditor.cpp
  tst_dataclient.h tst_dataclient.cpp
  qds/nullframepublisher.h qds/nullframepublisher.cpp
  qds/testlogger.h qds/testlogger.cpp
  qds/smartblocklcardmodule.cpp qds/smartblocklcardmodule.h
  qds/fakelcardmodule.h qds/fakelcardmodule.cpp
  qds/formulatest.h qds/formulatest.cpp
  qds/testdatasource.h qds/testdatasource.cpp
  qds/fakedatasource.h qds/fakedatasource.cpp
  qds/fakedatablocksink.h qds/fakedatablocksink.cpp
  qds/fakeactivedatasource.h qds/fakeactivedatasource.cpp
  qds/failoncedatasource.h qds/failoncedatasource.cpp
  qds/failingdatasource.h
  qds/countergenerator.h qds/countergenerator.cpp
  qds/generatordatasource.h qds/generatordatasource.cpp
  qds/failoncearchivewriter.h qds/failoncearchivewriter.cpp
  qds/failingarchivewriter.h
  qds/fakeschedulerclock.h qds/fakeschedulerclock.cpp
  qds/fakeclock.h qds/fakeclock.cpp

)

target_include_directories(DataClient
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/qds
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
    Hardware
    DataArchive
    SignalStorage
    Logger
    DataSettings
    DataWidgets
    DataClient
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
)

add_test(NAME unit_tests COMMAND unit_tests)
