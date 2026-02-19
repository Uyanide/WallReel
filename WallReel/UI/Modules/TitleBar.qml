import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string title: ""
    // start from 0
    property int index: 0
    property int totalCount: 0
    property int maxTitleLength: 50

    Label {
        text: (root.index + 1) + " / " + root.totalCount
        font.pixelSize: 12
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
    }

    Label {
        text: root.title.length > root.maxTitleLength ? root.title.substring(0, root.maxTitleLength) + "..." : root.title
        font.pixelSize: 12
        anchors.centerIn: parent
    }

}
