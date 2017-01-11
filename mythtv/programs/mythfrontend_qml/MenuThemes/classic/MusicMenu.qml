import QtQuick 2.0

ListModel
{
    id: musicMenu
    property string logo: "title/title_music.png"
    property string title: "Music Menu"

    ListElement
    {
        menutext: "Play Music"
        loaderSource:"playmusic.qml"
        waterMark: "watermark/music.png"
    }
    ListElement
    {
        menutext: "Play Radio Streams"
        loaderSource: "RadioPlayer.qml"
        waterMark: "watermark/radio.png"
    }
    ListElement
    {
        menutext:"Select Music"
        loaderSource: "selectmusic.qml"
        waterMark: "watermark/music.png"
    }
    ListElement
    {
        menutext: "Import CD"
        loaderSource: "ripmusic.qml"
        waterMark: "watermark/cd_rip.png"
    }
    ListElement
    {
        menutext:"Import Music"
        loaderSource: "importmusic.qml"
        waterMark: "watermark/music.png"
    }
    ListElement
    {
        menutext:"Scan For New  Music"
        loaderSource: "scanmusic.qml"
        waterMark: "watermark/music.png"
    }
    ListElement
    {
        menutext:"Eject CD"
        loaderSource: ""
        waterMark: "watermark/dvd_eject.png"
    }
    ListElement
    {
        menutext:"Music Settings"
        loaderSource: "musicsettings.qml"
        waterMark: "watermark/music_settings.png"
    }
}
