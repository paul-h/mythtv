import QtQuick 2.0

ListModel
{
    id: musicMenu

    ListElement
    {
        menutext: "Play Music"
        loaderSource:"playmusic.qml"
        waterMark: "watermark/video.png"
    }
    ListElement
    {
        menutext: "Play Radio Streams"
        loaderSource: "RadioPlayer.qml"
        waterMark: "watermark/record.png"
    }
    ListElement
    {
        menutext:"Select Music"
        loaderSource: "selectmusic.qml"
        waterMark: "watermark/info.png"
    }
    ListElement
    {
        menutext: "Import CD"
        loaderSource: "ripmusic.qml"
        waterMark: "watermark/dvd.png"
    }
    ListElement
    {
        menutext:"Import Music"
        loaderSource: "importmusic.qml"
        waterMark: "watermark/tv.png"
    }
    ListElement
    {
        menutext:"Scan For New  Music"
        loaderSource: "scanmusic.qml"
        waterMark: "watermark/setup.png"
    }
    ListElement
    {
        menutext:"Eject CD"
        loaderSource: ""
        waterMark: "watermark/weather.png"
    }
    ListElement
    {
        menutext:"Music Settings"
        loaderSource: "musicsettings.qml"
        waterMark: "watermark/keys.png"
    }
}
