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
    // Carousel selection state
    readonly property int currentIndex: ImageModel.currentIndex
    // Image name
    readonly property string focusedName: ImageModel.focusedName
    //// Sort
    readonly property var availableSortTypes: ["None", "Name", "Date", "Size"]
    readonly property string selectedSortType: ImageModel.currentSortType
    readonly property bool isSortReverse: ImageModel.currentSortReverse
    //// Palette / Color
    readonly property var availablePalettes: PaletteManager.availablePalettes
    readonly property var selectedPalette: PaletteManager.selectedPalette // PaletteItem | null
    readonly property var availableColors: selectedPalette ? selectedPalette.colors : []
    readonly property var selectedColor: PaletteManager.selectedColor // ColorItem | null  (null means "auto")
    readonly property string colorName: PaletteManager.colorName
    readonly property string colorHex: PaletteManager.color
    readonly property color colorValue: PaletteManager.color
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
        ServiceManager.cancel();
    }

    function setCurrentIndex(index) {
        ImageModel.currentIndex = index;
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
        PaletteManager.selectedPalette = palette;
    }

    function selectColor(colorItem) {
        PaletteManager.selectedColor = colorItem;
    }

    function setSearchText(text) {
        ImageModel.setSearchText(text);
        if (currentIndex != 0)
            currentIndex = 0;

    }

}
