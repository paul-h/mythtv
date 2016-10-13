import QtQuick 2.0
import "base"

Item
{
    //property alias source: mediaplayer.source
    property alias defaultFocusItem: player
    property alias source: player.source

    x: 0; y: 0; width: parent.width; height: parent.height

    Keys.onEscapePressed: if (stack.depth > 1) { player.stop(); stack.pop(); } else Qt.quit();

    VideoPlayerQmlVLC
    {
        id: player
        anchors.fill: parent
    }
}
