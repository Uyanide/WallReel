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
    readonly property var availablePalettes: PaletteManager.availablePalettes
    property var selectedPalette: null // PaletteItem | null
    readonly property var availableColors: selectedPalette ? selectedPalette.colors : []
    property var selectedColor: null // ColorItem | null  (null means "auto")
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
        if (currentIndex != 0)
            currentIndex = 0;

    }

    onCurrentIndexChanged: () => {
        if (!isLoading)
            ImageModel.focusOnIndex(currentIndex);

    }
    Component.onCompleted: () => {
        if (!isLoading)
            ImageModel.focusOnIndex(currentIndex);

    }
    onSelectedPaletteChanged: () => {
        PaletteManager.setSelectedPalette(selectedPalette);
    }
    onSelectedColorChanged: () => {
        PaletteManager.setSelectedColor(selectedColor);
    }
}
