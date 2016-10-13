import QtQuick 2.0

Item
{
    id: screenBackground
    property string title: ""
    property bool   showImage: true
    property bool   showTitle: false
    property bool   showTime: true
    property bool   showVideo: false
    property bool   showTicker: false

    x: 0; y : 0; width: window.width; height: window.height

    // background image
    Image
    {
        id: background
        visible: showImage
        anchors.fill: parent
        source: themePath + "ui/background.png"
    }

    // background video
    VideoPlayerQmlVLC
    {
        id: videoPlayer
        anchors.fill: parent
        visible: showVideo
        loop: true;

        Component.onDestruction:
        {
            if (showVideo)
            {
                stop();
            }
        }
    }

    // screen title
    TitleText { text: title }

    // time/date text
    DigitalClock
    {
        x: xscale(950); y: yscale(20); width: xscale(480); height: yscale(40)
        format: "ddd MMM dd, HH:mm:ss"
        visible: showTime
    }

    Component.onCompleted:
    {
        if (showVideo)
            videoPlayer.source = themePath + "video/blue_background.mp4";
    }
}
