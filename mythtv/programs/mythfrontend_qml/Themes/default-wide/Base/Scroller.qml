import QtQuick 2.0

Rectangle
{
    id: root
    property alias text: text.text
    height: 30
    width: parent.width

    anchors.bottom: parent.bottom
    color: "#88888888"

    Text
    {
        id: text
        color: "white"
        font.pixelSize: 25
        anchors.verticalCenter: parent.verticalCenter

        NumberAnimation on x
        {
            from: root.width
            to: -text.width
            duration: text.width * 30
            loops: Animation.Infinite
            onFromChanged: restart()
        }
    }
}

