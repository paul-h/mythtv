import QtQuick 2.0

ListModel
{
    id: zoneminderMenu
    property string logo: "title/title_setup.png"

    ListElement
    {
        menutext: "Show Console"
        loaderSource:"ZMConsole.qml"
        waterMark: "watermark/zoneminder.png"
    }
    ListElement
    {
        menutext: "Show Live View"
        loaderSource: "ZMLiveView.qml"
        waterMark: "watermark/zoneminder.png"
    }
    ListElement
    {
        menutext:"Show Events"
        loaderSource: "ZMEvents.qml"
        waterMark: "watermark/zoneminder.png"
    }
}
