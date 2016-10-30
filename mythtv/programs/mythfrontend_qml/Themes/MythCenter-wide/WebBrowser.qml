import QtQuick 2.0
import QtWebKit 3.0
import "base"

Item
{
    property alias defaultFocusItem: browser
    property alias url: browser.url

    x: 0; y: 0; width: parent.width; height: parent.height

    Keys.onEscapePressed: if (stack.depth > 1) {stack.pop(); escapeSound.play();} else Qt.quit();

    Image
    {
        id: background
        source: themePath + "ui/background.png"
    }

    TitleText { text: "Web Browser" }

    WebView
    {
        id: browser
        x: 10; y: 50; width: parent.width - 20; height: parent.height - 60
        url: "http://www.bbc.co.uk"

        Keys.onEscapePressed: if (stack.depth > 1) {stack.pop(); escapeSound.play();} else Qt.quit();
    }

}

