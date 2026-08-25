cmake_minimum_required(VERSION 3.20)

project(DataArchive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(DataArchive SHARED
    archiveformat.h
    archivefile.h archivefile.cpp
    archivewriter.h archivewriter.cpp
    archivereader.h archivereader.cpp
    archivemanager.h archivemanager.cpp
    iarchivewriter.h
    nullarchivewriter.h nullarchivewriter.cpp
    testarchivewriter.h testarchivewriter.cpp
    failingarchivewriter.h
    failoncearchivewriter.h failoncearchivewriter.cpp
    archivedescription.h
    archivedescriptionwriter.h archivedescriptionwriter.cpp
    archivedescriptionbuilder.h archivedescriptionbuilder.cpp
)

target_include_directories(DataArchive
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(DataArchive
    PUBLIC
      Configuration
      SignalStorage
      Qt6::Core
)
