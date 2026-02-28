import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property var availableSortTypes: []
    property string selectedSortType: ""
    property bool isDescending: false

    signal sortTypeSelected(string sortType)
    signal isDescendingToggled(bool descending)

    implicitWidth: row.implicitWidth
    implicitHeight: row.implicitHeight

    RowLayout {
        id: row

        anchors.fill: parent
        spacing: 4

        Label {
            text: "Sort by"
            font.pixelSize: 12
        }

        ComboBox {
            id: sortCombo

            implicitWidth: 90
            model: root.availableSortTypes
            onActivated: (index) => {
                return root.sortTypeSelected(root.availableSortTypes[index]);
            }

            Binding {
                target: sortCombo
                property: "currentIndex"
                value: root.availableSortTypes.indexOf(root.selectedSortType)
            }

        }

        ToolButton {
            icon.name: root.isDescending ? "view-sort-descending" : "view-sort-ascending"
            icon.width: 16
            icon.height: 16
            focusPolicy: Qt.NoFocus
            onClicked: root.isDescendingToggled(!root.isDescending)
            ToolTip.visible: hovered
            ToolTip.delay: 600
            ToolTip.text: root.isDescending ? "Descending order" : "Ascending order"
        }

    }

}
