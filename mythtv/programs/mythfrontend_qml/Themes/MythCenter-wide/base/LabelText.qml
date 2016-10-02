import QtQuick 2.0

Item
{
    id: root
    property string text: ""
    property string fontFamily: "Droid Sans"
    property int    fontPixelSize: 22
    property bool   fontBold: true
    property color  fontColor: "#00ff00"
    property int    horizontalAlignment: Text.AlignLeft
    property int    verticalAlignment: Text.AlignVCenter
    property double shadowAlpha: 100 / 255
    property color  shadowColor: "#000000"
    property int    shadowXOffset: 0
    property int    shadowYOffset: 0

    x: 50; y : 0; width: 300; height: 50

    Text
    {
        id: shadow
        text: root.text
        //anchors.fill: parent
        font.family: root.fontFamily
        font.pixelSize: root.fontPixelSize
        font.bold: root.fontBold
        color: root.shadowColor
        opacity: root.shadowAlpha
        x: shadowXOffset; y: shadowYOffset; width: parent.width; height: parent.height
        horizontalAlignment: root.horizontalAlignment
        verticalAlignment: root.verticalAlignment
        visible: shadowXOffset != 0 || shadowYOffset != 0 ? true : false
    }

    Text
    {
        id: text
        text: root.text
        anchors.fill: parent
        font.family: root.fontFamily
        font.pixelSize: root.fontPixelSize
        font.bold: root.fontBold
        color: root.fontColor
        x: 0; y: 0;  width: parent.width; height: parent.height
        horizontalAlignment: root.horizontalAlignment
        verticalAlignment: root.verticalAlignment
    }
}

