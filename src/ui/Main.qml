import QtQuick
import QtQuick.Controls
import WallReel.Core
import WallReel.UI.Pages

ApplicationWindow {
    width: Config.windowWidth
    height: Config.windowHeight
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    visible: true
    title: qsTr("Hello World")

    LoadingScreen {
        visible: ImageModel.isLoading
        anchors.fill: parent
        currentValue: ImageModel.processedCount
        totalValue: ImageModel.totalCount
    }

    Loader {
        anchors.fill: parent
        active: !ImageModel.isLoading

        sourceComponent: CarouselScreen {
            visible: !ImageModel.isLoading
        }

    }

}
