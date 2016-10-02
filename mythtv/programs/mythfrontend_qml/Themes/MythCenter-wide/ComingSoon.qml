import QtQuick 2.0
import "base"

Item
{
    property alias defaultFocusItem: text

    x: 0; y: 0; width: parent.width; height: parent.height

    Image
    {
        id: background
        source: themePath + "ui/background.png"
    }

    Image
    {
        id: name
        y: 100
        source: "images/comingsoon.png"
        anchors.horizontalCenter: parent.horizontalCenter
    }

    TitleText
    {
        id: text
        text: "Coming Soon!!"
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter

        Keys.onEscapePressed: if (stack.depth > 1) {stack.pop(); escapeSound.play();} else Qt.quit();
    }
}
