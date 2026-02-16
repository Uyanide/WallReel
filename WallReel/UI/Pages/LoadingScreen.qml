import QtQuick
import QtQuick.Controls

Item {
    property int currentValue: 0
    property int totalValue: 100

    Label {
        anchors.bottom: loadingBar.top
        anchors.horizontalCenter: loadingBar.horizontalCenter
        anchors.bottomMargin: 0
        text: currentValue + "/" + totalValue
        font.pixelSize: 12
    }

    ProgressBar {
        id: loadingBar

        anchors.centerIn: parent
        width: parent.width * 0.8
        value: totalValue > 0 ? currentValue / totalValue : 0
    }

}
