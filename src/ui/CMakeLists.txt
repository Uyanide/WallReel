qt_add_qml_module(${UILIB_NAME}
    URI ${UIMODULE_URI}
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    QML_FILES
    Main.qml
    Pages/LoadingScreen.qml
    Pages/CarouselScreen.qml
    Modules/Carousel.qml
)
