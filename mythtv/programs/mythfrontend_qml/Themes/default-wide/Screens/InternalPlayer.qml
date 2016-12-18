import QtQuick 2.0
import Base 1.0

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

        onPlaybackEnded:
        {
            stop();
            stack.pop();
        }
    }
}
