cmake_minimum_required(VERSION 3.16)

project(WallReel VERSION 0.1 LANGUAGES CXX)

set(EXECUTABLE_NAME "wallreel")
set(CORELIB_NAME "wallreel-core")
set(UILIB_NAME "wallreel-ui")
set(COREMODULE_URI "WallReel.Core")
set(UIMODULE_URI "WallReel.UI")
set(MODULE_VERSION_MAJOR 1)
set(MODULE_VERSION_MINOR 0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DSUPPRESS_QRC_LOG)
endif()

configure_file(WallReel/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets QuickControls2 Concurrent Sql)

qt_standard_project_setup(REQUIRES 6.5)

qt_policy(SET QTP0004 NEW)

option(BUILD_TESTING "Build the testing tree." ON)
option(ADDRESS_SANITIZER "Enable Address Sanitizer for debugging." OFF)

add_subdirectory(WallReel/Core)
add_subdirectory(WallReel/UI)

if(ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

add_executable(${EXECUTABLE_NAME}
    WallReel/main.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(${EXECUTABLE_NAME} PROPERTIES

    # MACOSX_BUNDLE_GUI_IDENTIFIER com.example.wallpaper-carousel
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    ${CORELIB_NAME}
    ${CORELIB_NAME}plugin
    ${UILIB_NAME}
    ${UILIB_NAME}plugin
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/generated
)

include(GNUInstallDirs)
install(TARGETS ${EXECUTABLE_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_subdirectory(WallReel/Assets)
