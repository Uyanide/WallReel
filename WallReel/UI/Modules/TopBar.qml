import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property int currentIndex: 0
    property int totalCount: 0
    property string title: ""
    property int maxTitleLength: 50
    readonly property string searchText: searchBar.typedText
    property alias availableSortTypes: sortCtrl.availableSortTypes
    property alias selectedSortType: sortCtrl.selectedSortType
    property alias isSortReverse: sortCtrl.isReverse

    signal sortTypeSelected(string sortType)
    signal sortReverseToggled(bool reverse)
    signal searchDismissed()

    function requestSearchFocus() {
        searchBar.requestFocus();
    }

    implicitHeight: row.implicitHeight

    RowLayout {
        id: row

        anchors.fill: parent
        spacing: 8

        Label {
            text: (root.currentIndex + 1) + " / " + root.totalCount
            font.pixelSize: 12
            Layout.preferredWidth: implicitWidth
        }

        Label {
            text: root.title.length > root.maxTitleLength ? root.title.substring(0, root.maxTitleLength) + "…" : root.title
            font.pixelSize: 12
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        SearchBar {
            id: searchBar

            Layout.alignment: Qt.AlignVCenter
            onDismissed: root.searchDismissed()
        }

        SortControl {
            id: sortCtrl

            Layout.alignment: Qt.AlignVCenter
            onSortTypeSelected: (t) => {
                return root.sortTypeSelected(t);
            }
            onIsReverseToggled: (r) => {
                return root.sortReverseToggled(r);
            }
        }

    }

}
