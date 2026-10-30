cmake_minimum_required(VERSION 3.23)
project(configurationvalidation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(configurationvalidation SHARED
  preparedconfiguration.h
  configurationvalidator.cpp
  configurationvalidator.h
  imoduleconfigurationvalidator.h
  moduleconfigurationvalidatorregistry.h moduleconfigurationvalidatorregistry.cpp
)

target_include_directories(configurationvalidation
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
    ${CMAKE_CURRENT_SOURCE_DIR}/db
)

target_link_libraries(configurationvalidation
  PUBLIC
    Configuration
    Formulas
    SignalStorage
  PRIVATE
    SignalProcessor
)