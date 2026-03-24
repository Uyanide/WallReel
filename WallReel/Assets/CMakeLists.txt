qt_add_resources(${EXECUTABLE_NAME} "app_icons"
    PREFIX "/"
    FILES icon.svg
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/app.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    RENAME ${EXECUTABLE_NAME}.desktop
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/apply.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    RENAME ${EXECUTABLE_NAME}-apply.desktop
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/icon.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
    RENAME ${EXECUTABLE_NAME}.svg
)
