qt_add_qml_module(${UILIB_NAME}
    URI ${UIMODULE_URI}
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    QML_FILES
    Main.qml
    Pages/LoadingScreen.qml
    Pages/CarouselScreen.qml
    Providers/CarouselProvider.qml
    Modules/Carousel.qml
    Modules/TitleBar.qml
    Modules/SearchBar.qml
    Modules/SortControl.qml
    Modules/ColorControl.qml
    Modules/TopBar.qml
    Modules/BottomBar.qml
    Components/WRTextButton.qml
)
