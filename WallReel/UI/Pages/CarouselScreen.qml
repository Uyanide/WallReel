import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WallReel.Core
import WallReel.UI.Modules

Item {
    id: root

    Component.onCompleted: root.forceActiveFocus()
    Keys.onPressed: (e) => {
        if (e.key === Qt.Key_Slash) {
            topBar.requestSearchFocus();
        } else if (e.key === Qt.Key_Left) {
            if (carousel.currentIndex > 0)
                carousel.currentIndex--;

        } else if (e.key === Qt.Key_Right) {
            if (carousel.currentIndex < carousel.count - 1)
                carousel.currentIndex++;

        } else if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter)
            CarouselProvider.confirm();
        else if (e.key === Qt.Key_Escape)
            CarouselProvider.cancel();
        else
            e.accepted = false;
    }

    Connections {
        function onSearchDismissed() {
            root.forceActiveFocus();
        }

        target: topBar
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        TopBar {
            id: topBar

            Layout.fillWidth: true
            totalCount: carousel.count
            title: carousel.currentImageName
            availableSortTypes: CarouselProvider.availableSortTypes
            isSortDescending: CarouselProvider.sortDescending
            onSortTypeSelected: (t) => {
                return CarouselProvider.setSortType(t);
            }
            onSortDescendingToggled: (r) => {
                return CarouselProvider.setSortDescending(r);
            }
            onSearchTextChanged: () => {
                return CarouselProvider.setSearchText(searchText);
            }

            Binding {
                target: topBar
                property: "currentIndex"
                value: carousel.currentIndex
            }

            Binding {
                target: topBar
                property: "selectedSortType"
                value: CarouselProvider.sortType
            }

        }

        Carousel {
            id: carousel

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: CarouselProvider.imageModel
            itemWidth: CarouselProvider.imageWidth
            itemHeight: CarouselProvider.imageHeight
            focusedItemWidth: CarouselProvider.imageWidth * CarouselProvider.imageFocusScale
            focusedItemHeight: CarouselProvider.imageHeight * CarouselProvider.imageFocusScale

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
                // Fallthrough to Carousel
                onPressed: (e) => {
                    root.forceActiveFocus();
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
            Layout.fillWidth: true
            from: 0
            to: Math.max(0, carousel.count - 1)
            value: carousel.currentIndex
            onMoved: carousel.currentIndex = Math.round(value)
        }

        BottomBar {
            id: bottomBar

            Layout.fillWidth: true
            availablePalettes: CarouselProvider.availablePalettes
            availableColors: CarouselProvider.selectedPalette ? CarouselProvider.selectedPalette.colors : []
            onPaletteSelected: (p) => {
                return CarouselProvider.requestSelectPalette(p);
            }
            onColorSelected: (c) => {
                return CarouselProvider.requestSelectColor(c);
            }
            onRestoreClicked: CarouselProvider.restore()
            onConfirmClicked: CarouselProvider.confirm()
            onCancelClicked: CarouselProvider.cancel()

            Binding {
                target: bottomBar
                property: "selectedPalette"
                value: CarouselProvider.selectedPalette
            }

            Binding {
                target: bottomBar
                property: "selectedColor"
                value: CarouselProvider.selectedColor
            }

            Binding {
                target: bottomBar
                property: "colorName"
                value: CarouselProvider.colorName
            }

            Binding {
                target: bottomBar
                property: "colorHex"
                value: CarouselProvider.color
            }

            Binding {
                target: bottomBar
                property: "colorValue"
                value: CarouselProvider.color
            }

            Binding {
                target: bottomBar
                property: "actionsEnabled"
                value: !CarouselProvider.isProcessing
            }

        }

    }

    Connections {
        function onCurrentImageIdChanged() {
            CarouselProvider.setCurrentImageId(carousel.currentImageId);
        }

        function onCurrentIndexChanged() {
            CarouselProvider.setCurrentIndex(carousel.currentIndex);
        }

        target: carousel
    }

}
