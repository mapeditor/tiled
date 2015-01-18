import QtQuick 2.0

MouseArea {
    property var lastPos

    signal dragged(var dx, var dy)

    onPressed: lastPos = mapToItem(null, mouse.x, mouse.y)

    onPositionChanged: {
        var pos = mapToItem(null, mouse.x, mouse.y)
        var dx = pos.x - lastPos.x
        var dy = pos.y - lastPos.y

        dragged(dx, dy)

        lastPos = pos
    }
}
