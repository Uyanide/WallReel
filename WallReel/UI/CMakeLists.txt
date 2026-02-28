qt_add_qml_module(${UILIB_NAME}
    STATIC
    URI ${UIMODULE_URI}
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    QML_FILES
    Main.qml
)

qt_add_qml_module(${UILIB_NAME}_Pages
    STATIC
    URI "${UIMODULE_URI}.Pages"
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Pages
    QML_FILES
    Pages/LoadingScreen.qml
    Pages/CarouselScreen.qml
)

qt_add_qml_module(${UILIB_NAME}_Modules
    STATIC
    URI "${UIMODULE_URI}.Modules"
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Modules
    QML_FILES
    Modules/Carousel.qml
    Modules/TitleBar.qml
    Modules/SortControl.qml
    Modules/ColorControl.qml
    Modules/TopBar.qml
    Modules/BottomBar.qml
)
qt_add_qml_module(${UILIB_NAME}_Components
    STATIC
    URI "${UIMODULE_URI}.Components"
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Components
    QML_FILES
    Components/WRSearchBar.qml
    Components/WRTextButton.qml
)
