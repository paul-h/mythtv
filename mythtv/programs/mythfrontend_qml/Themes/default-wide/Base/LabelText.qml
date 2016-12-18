import QtQuick 2.0

Item
{
    id: root
    property string text: ""
    property string fontFamily: theme.labelFontFamily
    property int    fontPixelSize: xscale(theme.labelFontPixelSize)
    property bool   fontBold: theme.labelFontBold
    property color  fontColor: theme.labelFontColor
    property int    horizontalAlignment: Text.AlignLeft
    property int    verticalAlignment: Text.AlignVCenter
    property double shadowAlpha: theme.labelShadowAlpha
    property color  shadowColor: theme.labelShadowColor
    property int    shadowXOffset: theme.labelShadowXOffset
    property int    shadowYOffset: theme.labelShadowYOffset


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

