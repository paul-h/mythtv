import QtQuick 2.0

ListModel
{
    id: mainMenu
    property string logo: "title/title_tv.png"

    ListElement
    {
        menutext: "TV"
        loaderSource:"PictureByPicture.qml"
        waterMark: "watermark/tv.png"
    }
    ListElement
    {
        menutext: "Internet Video"
        loaderSource: "TestPage.qml"
        waterMark: "watermark/tv.png"
    }
    ListElement
    {
        menutext:"Music"
        loaderSource: "ThemedMenu.qml"
        waterMark: "watermark/info.png"
    }
    ListElement
    {
        menutext: "Videos"
        loaderSource: "VideosGrid.qml"
        waterMark: "watermark/dvd.png"
    }
    ListElement
    {
        menutext:"Images"
        loaderSource: "IconView.qml"
        waterMark: "watermark/tv.png"
    }
    ListElement
    {
        menutext:"Weather"
        loaderSource: "ComingSoon.qml"
        waterMark: "watermark/weather.png"
    }
    ListElement
    {
        menutext:"Weather Station"
        loaderSource: "ComingSoon.qml"
        waterMark: "watermark/weather.png"
    }
    ListElement
    {
        menutext:"News Feeds"
        loaderSource: "RSSFeeds.qml"
        waterMark: "watermark/news.png"
    }
    ListElement
    {
        menutext:"Web"
        loaderSource: "WebBrowser.qml"
        waterMark: "watermark/browser.png"
    }
    ListElement
    {
        menutext:"Archive Files"
        loaderSource: "ComingSoon.qml"
        waterMark: "watermark/weather.png"
    }
    ListElement
    {
        menutext:"ZoneMinder"
        loaderSource: "ThemedMenu.qml"
        waterMark: "watermark/zoneminder.png"
    }
    ListElement
    {
        menutext:"Setup"
        loaderSource: "ThemedMenu.qml"
        waterMark: "watermark/setup.png"
    }
}
