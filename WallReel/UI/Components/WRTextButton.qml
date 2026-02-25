import QtQuick
import QtQuick.Controls

Button {
    //// Inherited from Button:
    // bool enabled
    // var onClicked

    property alias displayedText: label.text
    property color foregroundColor: "#89b4fa"
    property color disabledColor: "#585b70"

    flat: true
    focusPolicy: Qt.NoFocus

    contentItem: Label {
        id: label

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: parent.enabled ? parent.foregroundColor : parent.disabledColor
    }

}
