import QtQuick 2.0
import Base 1.0

Item
{
    property var    defaultFocusItem: undefined

    property bool   oldShowTitle: false
    property string oldTitle: ""

    property bool   oldShowTicker: false
    property bool   oldShowTime: false
    property bool   oldShowVideo: false

    x: 0; y: 0; width: parent.width; height: parent.height

    Keys.onEscapePressed: if (stack.depth > 1) {escapeSound.play(); stack.pop();} else Qt.quit();

    function showTitle (show, newTitle)
    {
        screenBackground.setTitle(show, newTitle);
    }

    function showTicker(show)
    {
        screenBackground.showTicker = show;
    }

    function showTime(show)
    {
        screenBackground.showTime = show;
    }

    function showVideo(show)
    {
        screenBackground.showVideo = show;
    }

    Component.onCompleted:
    {
        oldShowTitle = screenBackground.showTitle;
        oldTitle = screenBackground.title;

        oldShowTicker = screenBackground.showTicker;
        oldShowTime = screenBackground.showTime;
        oldShowVideo = screenBackground.showVideo;
    }

    Component.onDestruction:
    {
        screenBackground.showTitle = oldShowTitle;
        screenBackground.title = oldTitle;

        screenBackground.showTicker = oldShowTicker;
        screenBackground.showTime = oldShowTime;
        screenBackground.showVideo = oldShowVideo;
    }
}
