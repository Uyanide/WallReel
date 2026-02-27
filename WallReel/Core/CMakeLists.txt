qt_add_qml_module(${CORELIB_NAME}
    URI ${COREMODULE_URI}
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    SOURCES
    Image/data.hpp Image/data.cpp
    Image/model.hpp Image/model.cpp
    Palette/data.hpp
    Palette/manager.hpp Palette/manager.cpp
    Palette/domcolor.hpp Palette/domcolor.cpp
    Palette/matchcolor.hpp Palette/matchcolor.cpp
    Config/data.hpp
    Config/manager.hpp Config/manager.cpp
    logger.hpp logger.cpp
    Service/manager.hpp
    Service/wallpaper.hpp Service/wallpaper.cpp
    appoptions.hpp appoptions.cpp
)

target_link_libraries(${CORELIB_NAME} PUBLIC
    Qt6::Quick
    Qt6::Widgets
    Qt6::QuickControls2
    Qt6::Concurrent
)

target_include_directories(${CORELIB_NAME} PUBLIC
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_CURRENT_LIST_DIR}
)
