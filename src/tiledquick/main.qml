import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import org.mapeditor.Tiled 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480

    minimumWidth: 480
    minimumHeight: 320

    title: qsTr("Tiled")

    TiledQuickPlugin {}

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("New...")
                shortcut: StandardKey.New
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("Open...")
                shortcut: StandardKey.Open
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("Save")
                shortcut: StandardKey.Save
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("Save As...")
                shortcut: StandardKey.SaveAs
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("Exit")
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: qsTr("Edit")
            MenuItem {
                text: qsTr("Cut")
                shortcut: StandardKey.Cut
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("Copy")
                shortcut: StandardKey.Copy
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("Paste")
                shortcut: StandardKey.Paste
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("Delete")
                shortcut: StandardKey.Delete
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: qsTr("Help")
            MenuItem {
                text: qsTr("About Tiled")
                onTriggered: Qt.quit();
            }
            MenuItem {
                text: qsTr("About Qt")
                onTriggered: Qt.quit();
            }
        }
    }

    Text {
        text: qsTr("Hello World")
        anchors.centerIn: parent
    }

    statusBar: StatusBar {
        RowLayout {
            Label { text: "Read Only" }
        }
    }
}
