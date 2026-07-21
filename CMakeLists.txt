cmake_minimum_required(VERSION 3.20)

project(QtDataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

# Configuration	описание системы
# LiveStorage	хранение текущих значений
# Publisher	формирование LiveData
# SubscriptionManager	управление подписками
# LiveScheduler	когда публиковать
# DataProtocol	формат сетевых пакетов
# UdpSender	отправка UDP
# UdpServer	приём и обработка UDP
# DataSource    подсистема получения данных

add_subdirectory(tests)
add_subdirectory(libs/configuration)
add_subdirectory(libs/livestorage)
add_subdirectory(libs/publisher)
add_subdirectory(libs/subscriptions)
add_subdirectory(libs/dataprotocol)
add_subdirectory(libs/livescheduler)
add_subdirectory(libs/udpsender)
add_subdirectory(libs/udpserver)
add_subdirectory(libs/datasource)