import QtQuick 2.0
import QtWebKit 3.0
import Base 1.0

BaseScreen
{
    defaultFocusItem: browser
    property alias url: browser.url

    Component.onCompleted:
    {
        showTitle(true, "Web Browser");
        showTime(false);
        showTicker(false);
    }

    WebView
    {
        id: browser
        x: 10; y: 50; width: parent.width - 20; height: parent.height - 60
        url: "http://www.bbc.co.uk"

        Keys.onEscapePressed: if (stack.depth > 1) {stack.pop(); escapeSound.play();} else Qt.quit();
    }
}

