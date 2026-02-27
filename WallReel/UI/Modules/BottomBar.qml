import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WallReel.UI.Components

Item {
    id: root

    property bool actionsEnabled: true
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
                id: restoreButton

                displayedText: "Restore"
                onClicked: root.restoreClicked()
                foregroundColor: "#fab387"

                Binding {
                    target: restoreButton
                    property: "enabled"
                    value: root.actionsEnabled
                }

            }

            WRTextButton {
                id: confirmButton

                displayedText: "Confirm"
                onClicked: root.confirmClicked()
                foregroundColor: "#a6e3a1"

                Binding {
                    target: confirmButton
                    property: "enabled"
                    value: root.actionsEnabled
                }

            }

            WRTextButton {
                displayedText: "Cancel"
                onClicked: root.cancelClicked()
                foregroundColor: "#f38ba8"
            }

        }

    }

}
