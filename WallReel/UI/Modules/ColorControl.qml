import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property var availablePalettes: []
    property var selectedPalette: null
    property var availableColors: [] // list of ColorItem { name, color }
    property var selectedColor: null // null = "Auto"
    property string colorName: "Auto"
    property string colorHex: ""
    property color colorValue: "transparent"

    signal paletteSelected(var palette)
    signal colorSelected(var colorItem)

    implicitWidth: row.implicitWidth
    implicitHeight: row.implicitHeight

    RowLayout {
        id: row

        anchors.fill: parent
        spacing: 6

        ComboBox {
            id: paletteCombo

            implicitWidth: 200
            // -1 means nothing selected
            currentIndex: -1
            displayText: currentIndex < 0 ? "— palette —" : currentText
            model: root.availablePalettes.map((p) => {
                return p.name;
            })
            onActivated: (idx) => {
                root.paletteSelected(idx >= 0 ? root.availablePalettes[idx] : null);
            }
        }

        ComboBox {
            id: colorCombo

            implicitWidth: 100
            enabled: root.availableColors.length > 0
            model: ["Auto"].concat(root.availableColors.map((c) => {
                return c.name;
            }))
            currentIndex: {
                if (!root.selectedColor)
                    return 0;

                const idx = root.availableColors.findIndex((c) => {
                    return c.name === root.selectedColor.name;
                });
                return idx >= 0 ? idx + 1 : 0;
            }
            onActivated: (idx) => {
                root.colorSelected(idx === 0 ? null : root.availableColors[idx - 1]);
            }
        }

        Rectangle {
            width: 14
            height: 14
            radius: 7
            color: root.colorValue
            border.color: palette.mid
            border.width: 1
        }

        Label {
            font.pixelSize: 11
            text: {
                if (root.colorHex.length > 0)
                    return root.colorName.length > 0 ? root.colorName + "  " + root.colorHex : root.colorHex;

                return root.colorName;
            }
            visible: root.colorName.length > 0 || root.colorHex.length > 0
        }

    }

}
