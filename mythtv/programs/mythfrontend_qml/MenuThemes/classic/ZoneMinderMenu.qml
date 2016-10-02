import QtQuick 2.0

ListModel
{
    id: zoneminderMenu
    ListElement
    {
        menutext: "Show Console"
        loaderSource:"ComingSoon.qml"
        waterMark: "watermark/zoneminder.png"
    }
    ListElement
    {
        menutext: "Show Live View"
        loaderSource: "ComingSoon.qml"
        waterMark: "watermark/zoneminder.png"
    }
    ListElement
    {
        menutext:"Show Events"
        loaderSource: "ComingSoon.qml"
        waterMark: "watermark/zoneminder.png"
    }
}
