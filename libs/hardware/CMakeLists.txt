cmake_minimum_required(VERSION 3.23)
project(hardware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hardware SHARED
    networkutils.h networkutils.cpp
    ihardwaremodule.h
    ilcardmodule.h

    ltr11module.h ltr11module.cpp
    ltr11configuration.h
    lcardreadresult.h
    ltr11configurationvalidator.h ltr11configurationvalidator.cpp
    ltr11configurationfactory.h ltr11configurationfactory.cpp
    lcarddiscovery.h lcarddiscovery.cpp
)

if(WIN32)

    set(LCARD_DIR "C:/Program Files (x86)/L-Card")
    target_include_directories(hardware
        PRIVATE
            "${LCARD_DIR}/ltr/include"
    )
    target_link_directories(hardware
        PRIVATE
            "${LCARD_DIR}/ltr/lib/msvc64"
    )
    target_link_libraries(hardware
        PRIVATE
            ltrapi.lib
            ltr11api.lib
            ltr12api.lib
            ltr22api.lib
            ltr24api.lib
            ltr25api.lib
            ltr27api.lib
            ltr35api.lib
            ltr41api.lib
            ltr42api.lib
            ltr43api.lib
            ltr51api.lib
            ltr114api.lib
    )

elseif(LINUX)

    target_include_directories(hardware
        PRIVATE
            /usr/include/ltr/include
    )
    target_link_directories(hardware
        PRIVATE
            /usr/lib
    )
    target_link_libraries(hardware
        PRIVATE
            ltrapi
            ltr11api
            ltr12api
            ltr22api
            ltr24api
            ltr25api
            ltr27api
            ltr35api
            ltr41api
            ltr42api
            ltr43api
            ltr51api
            ltr114api
    )

endif()

target_include_directories(hardware
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(hardware
    PUBLIC
        configuration
        logger
)