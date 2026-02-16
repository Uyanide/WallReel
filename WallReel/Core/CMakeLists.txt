qt_add_qml_module(${CORELIB_NAME}
    URI ${COREMODULE_URI}
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    SOURCES
    imagedata.hpp imagedata.cpp
    utils/logger.hpp utils/logger.cpp
    utils/misc.hpp
    configmgr.hpp configmgr.cpp
    imagemodel.hpp imagemodel.cpp
    imageprovider.hpp imageprovider.cpp
)

target_link_libraries(${CORELIB_NAME} PRIVATE
    Qt6::Quick
    Qt6::Widgets
    Qt6::QuickControls2
    Qt6::Concurrent
)

target_include_directories(${CORELIB_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/generated
)
