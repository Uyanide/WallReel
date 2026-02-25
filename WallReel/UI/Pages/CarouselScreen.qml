import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WallReel.UI.Modules
import WallReel.UI.Providers

Item {
    id: root

    Component.onCompleted: root.forceActiveFocus()
    Keys.onPressed: (e) => {
        if (e.key === Qt.Key_Slash) {
            topBar.requestSearchFocus();
        } else if (e.key === Qt.Key_Left) {
            if (provider.currentIndex > 0)
                provider.currentIndex--;

        } else if (e.key === Qt.Key_Right) {
            if (provider.currentIndex < carousel.count - 1)
                provider.currentIndex++;

        } else if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter)
            provider.confirm();
        else if (e.key === Qt.Key_Escape)
            provider.cancel();
        else
            e.accepted = false;
    }

    Connections {
        function onSearchDismissed() {
            root.forceActiveFocus();
        }

        target: topBar
    }

    CarouselProvider {
        id: provider
    }

    // ViewModel → Carousel sync (assignment, not binding, to avoid breakage)
    Connections {
        function onCurrentIndexChanged() {
            if (carousel.currentIndex !== provider.currentIndex)
                carousel.currentIndex = provider.currentIndex;

        }

        target: provider
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        TopBar {
            id: topBar

            Layout.fillWidth: true
            currentIndex: provider.currentIndex
            totalCount: carousel.count
            title: provider.focusedName
            availableSortTypes: provider.availableSortTypes
            selectedSortType: provider.selectedSortType
            isSortReverse: provider.isSortReverse
            onSortTypeSelected: (t) => {
                return provider.setSortType(t);
            }
            onSortReverseToggled: (r) => {
                return provider.setSortReverse(r);
            }
            onSearchTextChanged: () => {
                return provider.setSearchText(topBar.searchText);
            }
        }

        Carousel {
            id: carousel

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: provider.imageModel
            itemWidth: provider.imageWidth
            itemHeight: provider.imageHeight
            focusedItemWidth: provider.imageWidth * provider.imageFocusScale
            focusedItemHeight: provider.imageHeight * provider.imageFocusScale
            onCurrentIndexChanged: {
                if (provider.currentIndex !== currentIndex)
                    provider.currentIndex = currentIndex;

            }

            MouseArea {
                anchors.fill: parent
                onWheel: (e) => {
                    if (e.angleDelta.y > 0) {
                        if (provider.currentIndex > 0)
                            provider.currentIndex--;

                    } else if (e.angleDelta.y < 0) {
                        if (provider.currentIndex < carousel.count - 1)
                            provider.currentIndex++;

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
            value: provider.currentIndex
            onMoved: provider.currentIndex = Math.round(value)
        }

        BottomBar {
            Layout.fillWidth: true
            availablePalettes: provider.availablePalettes
            selectedPalette: provider.selectedPalette
            availableColors: provider.availableColors
            selectedColor: provider.selectedColor
            colorName: provider.colorName
            colorHex: provider.colorHex
            colorValue: provider.colorValue
            onPaletteSelected: (p) => {
                return provider.selectPalette(p);
            }
            onColorSelected: (c) => {
                return provider.selectColor(c);
            }
            onRestoreClicked: provider.restore()
            onConfirmClicked: provider.confirm()
            onCancelClicked: provider.cancel()
            actionsEnabled: !provider.isProcessing
        }

    }

}
