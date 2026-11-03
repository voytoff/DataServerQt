cmake_minimum_required(VERSION 3.20)

project(test_basetableview LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(test_basetableview
    main.cpp
    testmodule.h testmodule.cpp
    ../../apps/configeditor/dlg/calibrationdialog.cpp
    ../../apps/configeditor/dlg/calibrationdialog.h
    ../../apps/configeditor/dlg/calibrationdialog.ui
    ../../apps/configeditor/model/calibrationtablemodel.cpp
    ../../apps/configeditor/model/calibrationtablemodel.h
    ../../apps/configeditor/plot/calibrationplot.cpp
    ../../apps/configeditor/plot/calibrationplot.h

    ../../apps/dataviewer/archivechartworker.h
    ../../apps/dataviewer/archivechartworker.cpp
    ../../apps/dataviewer/archivechartcontroller.h
    ../../apps/dataviewer/archivechartcontroller.cpp
  )

target_include_directories(test_basetableview
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/datatypes
    ${CMAKE_CURRENT_SOURCE_DIR}/../../apps/configeditor
    ${CMAKE_CURRENT_SOURCE_DIR}/../../apps/configeditor/model
    ${CMAKE_CURRENT_SOURCE_DIR}/../../apps/configeditor/delegate
    ${CMAKE_CURRENT_SOURCE_DIR}/../../apps/configeditor/dlg
    ${CMAKE_CURRENT_SOURCE_DIR}/../../apps/configeditor/plot
    ${CMAKE_CURRENT_SOURCE_DIR}/../../apps/dataviewer
)


target_link_libraries(test_basetableview
  PRIVATE
    Configuration
    QCustomPlot
    DataWidgets
    DataArchive
    Qt6::Widgets
)