cmake_minimum_required(VERSION 3.20)

project(test_basetableview LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(test_basetableview
    main.cpp
    ../../apps/configeditor/dlg/calibrationdialog.cpp
    ../../apps/configeditor/dlg/calibrationdialog.h
    ../../apps/configeditor/dlg/calibrationdialog.ui
    ../../apps/configeditor/model/calibrationtablemodel.cpp
    ../../apps/configeditor/model/calibrationtablemodel.h
    ../../apps/configeditor/plot/calibrationplot.cpp
    ../../apps/configeditor/plot/calibrationplot.h
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
)


target_link_libraries(test_basetableview
  PRIVATE
    Configuration
    QCustomPlot
    DataWidgets
    Qt6::Widgets
)