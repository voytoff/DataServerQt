cmake_minimum_required(VERSION 3.23)

project(Hardware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Hardware SHARED
    networkutils.h networkutils.cpp

    ihardwaremodule.h
    ilcardmodule.h

    lcardmodule.h lcardmodule.cpp

    fakelcardmodule.h fakelcardmodule.cpp

    ltr11module.h ltr11module.cpp
    ltr11configuration.h
)

if(WIN32)

    set(LCARD_DIR "C:/Program Files (x86)/L-Card")

    target_include_directories(Hardware
        PRIVATE
            "${LCARD_DIR}/ltr/include"
    )

    target_link_directories(Hardware
        PRIVATE
            "${LCARD_DIR}/ltr/lib/msvc64"
    )

    target_link_libraries(Hardware
        PRIVATE
            ltrapi.lib
            ltr11api.lib
    )

elseif(LINUX)

    target_include_directories(Hardware
        PRIVATE
            /usr/include/ltr/include
    )

    target_link_directories(Hardware
        PRIVATE
            /usr/lib
    )

    target_link_libraries(Hardware
        PRIVATE
            ltrapi
            ltr11api
    )

endif()

target_include_directories(Hardware
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Hardware
    PUBLIC
        Configuration
)