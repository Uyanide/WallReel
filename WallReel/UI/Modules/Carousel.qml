import QtQuick

Item {
    id: root

    required property var model
    property int currentIndex: 0
    property int itemWidth: 300
    property int itemHeight: 400
    property int focusedItemWidth: 450
    property int focusedItemHeight: 600
    property int spacing: 0
    property int animDuration: 200
    property int count: view.count

    ListView {
        id: view

        model: root.model
        anchors.fill: parent
        orientation: ListView.Horizontal
        spacing: root.spacing
        highlightRangeMode: ListView.StrictlyEnforceRange
        snapMode: ListView.SnapToItem
        highlightMoveDuration: root.animDuration
        preferredHighlightBegin: (width - root.focusedItemWidth) / 2
        preferredHighlightEnd: (width + root.focusedItemWidth) / 2
        clip: true
        cacheBuffer: root.itemWidth * 3
        onCurrentIndexChanged: {
            if (root.currentIndex !== view.currentIndex)
                root.currentIndex = view.currentIndex;

        }
        Component.onCompleted: {
            view.currentIndex = root.currentIndex;
            view.forceActiveFocus();
        }

        Connections {
            function onCurrentIndexChanged() {
                if (view.currentIndex !== root.currentIndex)
                    view.currentIndex = root.currentIndex;

            }

            target: root
        }

        delegate: Item {
            id: delegateItem

            property bool isFocused: ListView.isCurrentItem

            width: isFocused ? root.focusedItemWidth : root.itemWidth
            height: view.height
            z: isFocused ? 100 : 1

            Rectangle {
                anchors.centerIn: parent
                width: parent.width
                height: delegateItem.isFocused ? root.focusedItemHeight : root.itemHeight
                color: "transparent"

                Image {
                    id: img

                    anchors.fill: parent
                    source: model.imgUrl
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    cache: true
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        view.currentIndex = index;
                        view.forceActiveFocus();
                    }
                }

                Behavior on height {
                    NumberAnimation {
                        duration: root.animDuration
                        easing.type: Easing.OutCubic
                    }

                }

            }

            Behavior on width {
                NumberAnimation {
                    duration: root.animDuration
                    easing.type: Easing.OutCubic
                }

            }

        }

    }

}
