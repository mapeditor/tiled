import QtQuick

MouseArea {
    property var lastDragPos

    signal dragged(var dx, var dy)

    hoverEnabled: true
    onPressed: function(mouse) {
        lastDragPos = mapToItem(null, mouse.x, mouse.y)
    }

    onPositionChanged: function(mouse) {
        if (!pressed)
            return;

        var pos = mapToItem(null, mouse.x, mouse.y)
        var dx = pos.x - lastDragPos.x
        var dy = pos.y - lastDragPos.y

        dragged(dx, dy)

        lastDragPos = pos
    }
}
