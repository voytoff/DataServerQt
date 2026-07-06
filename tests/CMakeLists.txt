cmake_minimum_required(VERSION 3.20)
project(test_common LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_common tst_test_common.cpp)

target_include_directories(test_common PRIVATE
    ../libs/configuration
    ../libs/livestorage
    ../libs/subscriptions
    ../libs/dataprotocol
)

target_link_libraries(test_common PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Configuration
    LiveStorage
    Subscriptions
    DataProtocol
)

add_test(NAME test_common COMMAND test_common)