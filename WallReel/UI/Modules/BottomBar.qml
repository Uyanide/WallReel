import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WallReel.UI.Components

Item {
    id: root

    required property bool actionsEnabled
    property alias availablePalettes: colorCtrl.availablePalettes
    property alias selectedPalette: colorCtrl.selectedPalette
    property alias availableColors: colorCtrl.availableColors
    property alias selectedColor: colorCtrl.selectedColor
    property alias colorName: colorCtrl.colorName
    property alias colorHex: colorCtrl.colorHex
    property alias colorValue: colorCtrl.colorValue

    signal paletteSelected(var palette)
    signal colorSelected(var colorItem)
    signal restoreClicked()
    signal confirmClicked()
    signal cancelClicked()

    implicitHeight: row.implicitHeight

    RowLayout {
        id: row

        anchors.fill: parent
        spacing: 12

        ColorControl {
            id: colorCtrl

            Layout.alignment: Qt.AlignVCenter
            onPaletteSelected: (p) => {
                return root.paletteSelected(p);
            }
            onColorSelected: (c) => {
                return root.colorSelected(c);
            }
        }

        // Flexible spacer
        Item {
            Layout.fillWidth: true
        }

        // Action buttons
        RowLayout {
            spacing: 20
            Layout.alignment: Qt.AlignVCenter

            WRTextButton {
                displayedText: "Restore"
                onClicked: root.restoreClicked()
                enabled: root.actionsEnabled
                foregroundColor: "#fab387"
            }

            WRTextButton {
                displayedText: "Confirm"
                onClicked: root.confirmClicked()
                enabled: root.actionsEnabled
                foregroundColor: "#a6e3a1"
            }

            WRTextButton {
                displayedText: "Cancel"
                onClicked: root.cancelClicked()
                foregroundColor: "#f38ba8"
            }

        }

    }

}
