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
            }
            MenuItem {
                text: qsTr("Open...")
                shortcut: StandardKey.Open
            }
            MenuItem {
                text: qsTr("Save")
                shortcut: StandardKey.Save
            }
            MenuItem {
                text: qsTr("Save As...")
                shortcut: StandardKey.SaveAs
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
            }
            MenuItem {
                text: qsTr("Copy")
                shortcut: StandardKey.Copy
            }
            MenuItem {
                text: qsTr("Paste")
                shortcut: StandardKey.Paste
            }
            MenuItem {
                text: qsTr("Delete")
                shortcut: StandardKey.Delete
            }
        }
        Menu {
            title: qsTr("Help")
            MenuItem {
                text: qsTr("About Tiled")
            }
            MenuItem {
                text: qsTr("About Qt")
            }
        }
    }

    toolBar: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                iconName: "document-new"
            }
            Item { Layout.fillWidth: true }
            CheckBox {
                text: "Enabled"
                checked: true
                Layout.alignment: Qt.AlignRight
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
