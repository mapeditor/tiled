import QtQuick

Rectangle {
    anchors.fill: parent
    color: "lime"

    Rectangle {
        anchors.centerIn: parent
        width: 200; height: 200
        color: "magenta"

        Text {
            anchors.centerIn: parent
            text: "TEST"
            color: "white"
            font.bold: true
        }
    }
}
