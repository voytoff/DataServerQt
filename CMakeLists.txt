cmake_minimum_required(VERSION 3.23)

project(DataServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_EXAMPLES OFF)
set(BUILD_TESTS OFF)
set(USE_NO_MD5 ON)

#add_subdirectory(libs/livestorage)
#add_subdirectory(libs/livescheduler)
#add_subdirectory(libs/scheduler)
#add_subdirectory(libs/hardwarefactory)
#add_subdirectory(libs/moduleloader)

add_subdirectory(libs/configuration)
add_subdirectory(libs/publisher)
add_subdirectory(libs/subscriptions)
add_subdirectory(libs/dataprotocol)
add_subdirectory(libs/udpsender)
add_subdirectory(libs/udpserver)
add_subdirectory(libs/datasource)
add_subdirectory(libs/core)
add_subdirectory(libs/engine)
add_subdirectory(libs/hardware)
add_subdirectory(libs/packetdispatcher)
add_subdirectory(libs/dataarchive)
add_subdirectory(libs/signalstorage)
add_subdirectory(libs/signalprocessor)
add_subdirectory(libs/formulas)
add_subdirectory(libs/dataserverruntime)
add_subdirectory(libs/logger)
add_subdirectory(libs/dataclient)
add_subdirectory(libs/datawidgets)
add_subdirectory(libs/datasettings)
add_subdirectory(libs/configurationvalidation)
add_subdirectory(libs/modulevalidators)
add_subdirectory(libs/signalprocessing)

add_subdirectory(3rdparty/qcustomplot)

# zlib
# git clone --depth 1 https://github.com/madler/zlib.git
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(3rdparty/zlib)

# Параметры для стандартного FindZLIB.
set(ZLIB_INCLUDE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/zlib"
  CACHE PATH "" FORCE
)

set(ZLIB_LIBRARY
  zlibstatic
  CACHE STRING "" FORCE
)

if(NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

# libxlsxwriter
# git clone --depth 1 https://github.com/jmcnamara/libxlsxwriter.git
set(BUILD_SHARED_LIBS OFF)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(USE_NO_MD5 ON CACHE BOOL "" FORCE)

add_subdirectory(3rdparty/libxlsxwriter)

enable_testing()
add_subdirectory(tests)
add_subdirectory(tests/test_basetableview)

add_subdirectory(libs/dataexport)

add_subdirectory(apps/dataserver)
add_subdirectory(apps/configeditor)
add_subdirectory(apps/datamonitor)
add_subdirectory(apps/dataviewer)
