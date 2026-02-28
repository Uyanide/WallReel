import QtQuick
import QtQuick.Controls
import WallReel.Core
import WallReel.UI.Pages

ApplicationWindow {
    width: CarouselProvider.windowWidth
    height: CarouselProvider.windowHeight
    // minimumWidth: width
    // maximumWidth: width
    // minimumHeight: height
    // maximumHeight: height
    visible: true
    title: qsTr("WallReel")

    LoadingScreen {
        visible: CarouselProvider.isLoading
        anchors.fill: parent
        currentValue: CarouselProvider.processedCount
        totalValue: CarouselProvider.totalCount
    }

    Loader {
        anchors.fill: parent
        active: !CarouselProvider.isLoading

        sourceComponent: CarouselScreen {
        }

    }

}
