import QtQuick
import WallReel.Core

QtObject {
    id: root

    //// Image model
    readonly property var imageModel: ImageModel
    readonly property bool isLoading: ImageModel.isLoading
    readonly property int processedCount: ImageModel.processedCount
    readonly property int totalCount: ImageModel.totalCount
    // Image display dimensions (from Config)
    readonly property int imageWidth: Config.imageWidth
    readonly property int imageHeight: Config.imageHeight
    readonly property real imageFocusScale: Config.imageFocusScale
    // Shared carousel selection state
    property int currentIndex: 0
    // Image name
    readonly property string focusedName: ImageModel.focusedName
    //// Sort
    readonly property var availableSortTypes: ["None", "Name", "Date", "Size"]
    property string selectedSortType: ImageModel.currentSortType
    property bool isSortReverse: ImageModel.currentSortReverse
    //// Palette / Color
    readonly property var availablePalettes: []
    property var selectedPalette: null // PaletteItem | null
    readonly property var availableColors: selectedPalette ? selectedPalette.colors : []
    property var selectedColor: null // ColorItem | null  (null means "auto")
    readonly property string colorName: selectedColor ? selectedColor.name : "Auto"
    readonly property string colorHex: selectedColor ? selectedColor.color.toString().toUpperCase() : ""
    readonly property color colorValue: selectedColor ? selectedColor.color : "transparent"
    //// Actions state
    readonly property bool isProcessing: ServiceManager.isProcessing

    // Actions
    function confirm() {
        ServiceManager.selectWallpaper(currentIndex);
    }

    function restore() {
        ServiceManager.restore();
    }

    function cancel() {
        Qt.quit();
    }

    function focusSearch() {
        searchBar.requestFocus();
    }

    function setSortType(type) {
        ImageModel.currentSortType = type;
    }

    function setSortReverse(reverse) {
        ImageModel.currentSortReverse = reverse;
    }

    function selectPalette(palette) {
        selectedPalette = palette;
        selectedColor = null; // reset color when palette changes
    }

    function selectColor(colorItem) {
        selectedColor = colorItem;
    }

    function setSearchText(text) {
        ImageModel.setSearchText(text);
        currentIndex = 0; // reset index when search text changes
    }

    onCurrentIndexChanged: () => {
        if (!isLoading) {
            ServiceManager.previewWallpaper(currentIndex);
            ImageModel.focusOnIndex(currentIndex);
        }
    }
    Component.onCompleted: () => {
        if (!isLoading) {
            ServiceManager.previewWallpaper(currentIndex);
            ImageModel.focusOnIndex(currentIndex);
        }
    }
}
