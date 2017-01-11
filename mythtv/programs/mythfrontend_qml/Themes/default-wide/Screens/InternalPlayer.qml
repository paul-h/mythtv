import QtQuick 2.0
import Base 1.0

BaseScreen
{
    defaultFocusItem: player
    property alias source: player.source

    Component.onCompleted:
    {
        showTitle(false, "");
        showTime(false);
        showTicker(false);
        showVideo(false);
    }

    Keys.onEscapePressed:  player.stop();

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
