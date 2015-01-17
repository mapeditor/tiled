import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import org.mapeditor.Tiled 1.0 as Tiled
import Qt.labs.settings 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480

    minimumWidth: 480
    minimumHeight: 320

    title: qsTr("Tiled Quick")

    FileDialog {
        id: fileDialog
        nameFilters: [ "TMX files (*.tmx)", "All files (*)" ]
        onAccepted: mapLoader.source = fileDialog.fileUrl
    }

    MessageDialog {
        id: aboutBox
        title: "About Tiled Quick"
        text: "This is an experimental Qt Quick version of Tiled,\na generic 2D map editor"
        icon: StandardIcon.Information
    }

    Settings {
        property alias mapSource: mapLoader.source
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
//            MenuItem {
//                text: qsTr("New...")
//                shortcut: StandardKey.New
//            }
            MenuItem {
                text: qsTr("Open...")
                shortcut: StandardKey.Open
                onTriggered: {
                    fileDialog.open()
                }
            }
//            MenuItem {
//                text: qsTr("Save")
//                shortcut: StandardKey.Save
//            }
//            MenuItem {
//                text: qsTr("Save As...")
//                shortcut: StandardKey.SaveAs
//            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Exit")
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit();
            }
        }
//        Menu {
//            title: qsTr("Edit")
//            MenuItem {
//                text: qsTr("Cut")
//                shortcut: StandardKey.Cut
//            }
//            MenuItem {
//                text: qsTr("Copy")
//                shortcut: StandardKey.Copy
//            }
//            MenuItem {
//                text: qsTr("Paste")
//                shortcut: StandardKey.Paste
//            }
//            MenuItem {
//                text: qsTr("Delete")
//                shortcut: StandardKey.Delete
//            }
//            MenuSeparator {}
//            MenuItem {
//                text: qsTr("Select All")
//                shortcut: StandardKey.SelectAll
//            }
//        }
        Menu {
            title: qsTr("Help")
            MenuItem {
                text: qsTr("About Tiled Quick")
                onTriggered: aboutBox.open()
            }
        }
    }

//    toolBar: ToolBar {
//        RowLayout {
//            anchors.fill: parent
//            ToolButton {
//                iconName: "document-new"
//            }
//            Item { Layout.fillWidth: true }
//            CheckBox {
//                text: "Enabled"
//                checked: true
//                Layout.alignment: Qt.AlignRight
//            }
//        }
//    }

    Tiled.MapLoader {
        id: mapLoader
    }

    ScrollView {
        anchors.fill: parent

        Flickable {
            id: flickable

            anchors.fill: parent

            contentWidth: mapItem.width
            contentHeight: mapItem.height

            pixelAligned: true

            Tiled.MapItem {
                id: mapItem
                map: mapLoader.map
                visibleArea: Qt.rect(flickable.contentX,
                                     flickable.contentY,
                                     flickable.width,
                                     flickable.height);
            }
        }
    }

    Text {
        text: mapLoader.status === Tiled.MapLoader.Null ? qsTr("No map file loaded") : mapLoader.error
        anchors.centerIn: parent
    }

//    statusBar: StatusBar {
//        RowLayout {
//            Label { text: "Read Only" }
//        }
//    }
}
