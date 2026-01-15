cmake_minimum_required(VERSION 3.16)

project(Wallpaper_Carousel VERSION 1.0.2 LANGUAGES CXX)

set(EXECUTABLE_NAME "wallpaper-carousel")

configure_file(src/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS src/designer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
    src/designer/main_window.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${EXECUTABLE_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/utils.h
        src/images_carousel.h src/images_carousel.cpp src/designer/images_carousel.ui
        src/config.h src/config.cpp
        src/logger.h src/logger.cpp
        src/loading_indicator.h src/loading_indicator.cpp src/designer/loading_indicator.ui
        src/image_item.h src/image_item.cpp
    )

# Define target properties for Android with Qt 6 as:
# set_property(TARGET wallpaper_chooser APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
# ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${EXECUTABLE_NAME} SHARED
            ${PROJECT_SOURCES}
        )

    # Define properties for Android with Qt 5 after find_package() calls as:
    # set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${EXECUTABLE_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(${EXECUTABLE_NAME} PRIVATE src ${CMAKE_BINARY_DIR}/generated)

# if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
# target_compile_definitions(wallpaper_chooser PRIVATE
# GENERAL_LOGGER_DISABLED
# )
# endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.wallpaper_chooser)
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${EXECUTABLE_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/app/${EXECUTABLE_NAME}.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${EXECUTABLE_NAME})
endif()
