qt_add_resources(${EXECUTABLE_NAME} "app_icons"
    PREFIX "/"
    FILES
    ${EXECUTABLE_NAME}.svg
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/${EXECUTABLE_NAME}.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/${EXECUTABLE_NAME}.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)
