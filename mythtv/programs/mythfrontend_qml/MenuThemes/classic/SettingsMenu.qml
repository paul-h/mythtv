import QtQuick 2.0

ListModel
{
    id: settingsMenu
    ListElement
    {
        menutext: "Channel Editor"
        loaderSource:"ChannelEditor.qml"
        waterMark: "watermark/keys.png"
    }
    ListElement
    {
        menutext: "General Settings"
        loaderSource: "SettingEditor.qml"
        waterMark: "watermark/keys.png"
    }
}
