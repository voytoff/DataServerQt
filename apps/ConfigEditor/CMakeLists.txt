cmake_minimum_required(VERSION 3.23)
project(configeditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS           Core Widgets Sql Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Widgets Sql Svg)

set(CONFIGEDITOR_RESOURCE_FILES res.qrc)
qt6_add_resources(CONFIGEDITOR_PROJECT_RESOURCES ${CONFIGEDITOR_RESOURCE_FILES})
set(APP_ICON_RESOURCE_WINDOWS "appicon.rc")

qt_add_executable(configeditor
    ${CONFIGEDITOR_PROJECT_RESOURCES}
    ${APP_ICON_RESOURCE_WINDOWS}
    main.cpp
    treeitemdata.h
    mainwindow.h mainwindow.cpp
    configurationeditor.h configurationeditor.cpp
    model/configurationtreemodel.h model/configurationtreemodel.cpp
    treeitem.h treeitem.cpp
    model/configurationpropertiesmodel.h model/configurationpropertiesmodel.cpp
    delegate/configurationpropertiesdelegate.h delegate/configurationpropertiesdelegate.cpp
    model/configurationsignalmodel.h model/configurationsignalmodel.cpp
    model/configurationsignalpropertiesmodel.h model/configurationsignalpropertiesmodel.cpp
    delegate/configurationsignalpropertiesdelegate.h delegate/configurationsignalpropertiesdelegate.cpp
    dlg/configurationsignaldialog.h dlg/configurationsignaldialog.cpp
    model/sensortypemodel.h model/sensortypemodel.cpp
    delegate/basecomboboxdelegate.h delegate/basecomboboxdelegate.cpp
    model/sensormodel.h model/sensormodel.cpp
    dlg/sensordialog.h dlg/sensordialog.cpp
    model/sensorpropertiesmodel.h model/sensorpropertiesmodel.cpp
    model/calibrationmodel.h model/calibrationmodel.cpp
    model/calibrationtablemodel.h model/calibrationtablemodel.cpp
    dlg/calibrationdialog.h dlg/calibrationdialog.cpp dlg/calibrationdialog.ui
    plot/calibrationplot.h plot/calibrationplot.cpp
    dlg/configurationdialog.h dlg/configurationdialog.cpp dlg/configurationdialog.ui
)
#[[
if(WIN32)
  add_custom_command(
    TARGET configeditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:configeditor>
      $<TARGET_FILE_DIR:configeditor>
    COMMAND_EXPAND_LISTS
  )
endif()
]]
target_include_directories(configeditor
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/datatypes
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/delegate
    ${CMAKE_CURRENT_SOURCE_DIR}/dlg
    ${CMAKE_CURRENT_SOURCE_DIR}/plot
)

target_link_libraries(configeditor
    PRIVATE
        Configuration
        DataSettings
        DataWidgets
        QCustomPlot
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
)