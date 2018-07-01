import QtQuick 2.0
import QtQuick.Controls 1.4
import QtWebEngine 1.3
import Base 1.0

BaseScreen
{
    id: root
    defaultFocusItem: browser
    property alias url: browser.url
    property bool fullscreen: true

    Component.onCompleted:
    {
        showTitle(!fullscreen, "Web Browser");
        showTime(false);
        showTicker(false);
        showMouse(true);
    }

    Component.onDestruction: showMouse(false)

    Action
    {
        shortcut: "Escape"
        onTriggered: if (stack.depth > 1) {stack.pop(); escapeSound.play();} else Qt.quit();
    }

    Action
    {
        shortcut: "F8"
        onTriggered: { root.fullscreen = !root.fullscreen; showTitle(!root.fullscreen, "Web Browser"); }
    }

    WebEngineView
    {
        id: browser
        x: root.fullscreen ? 0 : xscale(10);
        y: root.fullscreen ? 0 : yscale(50);
        width: root.fullscreen ? parent.width : parent.width - xscale(20);
        height: root.fullscreen ? parent.height : parent.height - yscale(60)
        url: "https://www.google.co.uk"
        settings.pluginsEnabled : true
    }
}

