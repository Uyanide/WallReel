import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    readonly property string typedText: field.text

    // Emitted when leave the field.
    signal dismissed()

    function requestFocus() {
        field.forceActiveFocus();
    }

    implicitWidth: row.implicitWidth
    implicitHeight: row.implicitHeight

    RowLayout {
        id: row

        anchors.fill: parent
        spacing: 2

        ToolButton {
            icon.name: "edit-find"
            icon.width: 16
            icon.height: 16
            focusPolicy: Qt.NoFocus
            onClicked: root.requestFocus()
            ToolTip.visible: hovered
            ToolTip.delay: 600
            ToolTip.text: "Search  (/)"
        }

        TextField {
            id: field

            Layout.preferredWidth: activeFocus || text.length > 0 ? 180 : 0
            clip: true
            placeholderText: "Search…"
            leftPadding: 6
            rightPadding: 6
            Keys.onReturnPressed: {
                focus = false;
                root.dismissed();
            }
            Keys.onEscapePressed: {
                text = "";
                focus = false;
                root.dismissed();
            }

            Behavior on Layout.preferredWidth {
                NumberAnimation {
                    duration: 160
                    easing.type: Easing.OutCubic
                }

            }

        }

        ToolButton {
            icon.name: "edit-clear"
            icon.width: 16
            icon.height: 16
            focusPolicy: Qt.NoFocus
            visible: field.text.length > 0
            onClicked: {
                field.text = "";
                field.forceActiveFocus();
            }
            ToolTip.visible: hovered
            ToolTip.delay: 600
            ToolTip.text: "Clear"
        }

    }

}
