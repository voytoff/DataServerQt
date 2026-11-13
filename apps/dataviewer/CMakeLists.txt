cmake_minimum_required(VERSION 3.23)
project(dataviewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS           Core Widgets Sql Svg Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Widgets Sql Svg Charts)

set(DATAVIEWER_RESOURCE_FILES res.qrc)
qt6_add_resources(DATAVIEWER_PROJECT_RESOURCES ${DATAVIEWER_RESOURCE_FILES})
set(APP_ICON_RESOURCE_WINDOWS "appicon.rc")

qt_add_executable(dataviewer
  ${DATAVIEWER_PROJECT_RESOURCES}
  ${APP_ICON_RESOURCE_WINDOWS}
  main.cpp
  dataviewermainwindow.h dataviewermainwindow.cpp
  viewersettingsloader.h viewersettingsloader.cpp
)

if(WIN32)

  get_target_property(
    QT_QMAKE_EXECUTABLE
    Qt6::qmake
    IMPORTED_LOCATION
  )

  get_filename_component(
    QT_BIN_DIR
    "${QT_QMAKE_EXECUTABLE}"
    DIRECTORY
  )

  find_program(
    WINDEPLOYQT_EXECUTABLE
   +
   +*NAMES windeployqt
    HINTS "${QT_BIN_DIR}"
    REQUIRED
  )

  add_custom_command(
    TARGET dataviewer
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:dataviewer>
      $<TARGET_FILE_DIR:dataviewer>

    COMMAND "${WINDEPLOYQT_EXECUTABLE}"
      --dir "$<TARGET_FILE_DIR:dataviewer>"
      "$<TARGET_FILE:dataviewer>"

    COMMAND_EXPAND_LISTS
    VERBATIM
  )

endif()

target_include_directories(dataviewer
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/datatypes
)

target_link_libraries(dataviewer
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Charts
    configuration
    datasettings
    datawidgets
    signalprocessing
    dataarchive
)