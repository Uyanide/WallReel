import QtQuick
import QtQuick.Controls

ToolButton {
    id: reloadBtn

    property bool isLoading: false

    icon.name: "view-refresh"
    icon.width: 16
    icon.height: 16
    focusPolicy: Qt.NoFocus
    ToolTip.visible: hovered
    ToolTip.delay: 600
    ToolTip.text: "Reload from disk"
    enabled: !isLoading
}
