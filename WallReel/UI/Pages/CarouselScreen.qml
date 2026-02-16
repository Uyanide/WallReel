import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WallReel.Core
import WallReel.UI.Modules

Item {
    id: root

    Keys.onPressed: (e) => {
        if (e.key === Qt.Key_Left) {
            if (carousel.currentIndex > 0)
                carousel.currentIndex--;

        } else if (e.key === Qt.Key_Right) {
            if (carousel.currentIndex < carousel.count - 1)
                carousel.currentIndex++;

        } else if (e.key === Qt.Key_Escape)
            Qt.quit();
        else if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter)
            ImageModel.confirm(carousel.currentIndex);
        else
            e.accepted = false;
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Label {
            text: (ImageModel.dataAt(carousel.currentIndex, "imgName") ?? "") + " (" + (carousel.currentIndex + 1) + "/" + carousel.count + ")"
            font.pixelSize: 12
            Layout.alignment: Qt.AlignHCenter
        }

        Carousel {
            id: carousel

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: ImageModel
            itemWidth: Config.imageWidth
            itemHeight: Config.imageWidth / Config.aspectRatio
            focusedItemWidth: Config.imageFocusWidth
            focusedItemHeight: Config.imageFocusWidth / Config.aspectRatio

            MouseArea {
                anchors.fill: parent
                onWheel: (e) => {
                    if (e.angleDelta.y > 0) {
                        if (carousel.currentIndex > 0)
                            carousel.currentIndex--;

                    } else if (e.angleDelta.y < 0) {
                        if (carousel.currentIndex < carousel.count - 1)
                            carousel.currentIndex++;

                    }
                }
                onPressed: (e) => {
                    carousel.forceActiveFocus();
                    e.accepted = false;
                }
                onPositionChanged: (e) => {
                    e.accepted = false;
                }
                onReleased: (e) => {
                    e.accepted = false;
                }
            }

        }

        Slider {
            id: progressBar

            Layout.fillWidth: true
            from: 0
            to: carousel.count - 1
            value: carousel.currentIndex
            onMoved: {
                if (carousel.currentIndex !== value)
                    carousel.currentIndex = value;

            }
        }

    }

}
