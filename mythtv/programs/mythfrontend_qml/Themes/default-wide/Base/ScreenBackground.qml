import QtQuick 2.0

Item
{
    id: screenBackground
    property string title: ""
    property alias showImage: background.visible
    property alias showTitle: screenTitle.visible
    property alias showTime: time.visible
    property alias showVideo: videoPlayer.visible
    property alias showTicker: ticker.visible

    function setTitle (show, newTitle)
    {
        screenTitle.visible = show;
        screenTitle.text = newTitle;
    }

    function setTicker(show, newText)
    {
        ticker.visible = show;
        ticker.text = newText;
    }

    function addTickerItem(id, item)
    {
        ticker.model.append({"id": id, "text": item})
    }

    function muteAudio(mute)
    {
        if (videoPlayer.visible && videoPlayer.mute != mute)
            videoPlayer.toggleMute()
    }

    x: 0; y : 0; width: window.width; height: window.height

    // background image
    Image
    {
        id: background
        visible: true
        anchors.fill: parent
        source: settings.themePath + "ui/background.png"
    }

    // background video
    VideoPlayerQmlVLC
    {
        id: videoPlayer
        anchors.fill: parent
        visible: false
        loop: true;

        Component.onCompleted:
        {
            if (showVideo)
            {
                setLoopMode(true);
            }
        }

        Component.onDestruction:
        {
            if (showVideo)
            {
                stop();
            }
        }
    }

    // screen title
    TitleText
    {
        id: screenTitle
        text: title
        visible : true
    }

    // time/date text
    DigitalClock
    {
        id: time
        x: xscale(750); y: yscale(0); width: xscale(500); height: yscale(50)
        format: "ddd MMM dd, HH:mm:ss"
        visible: true
    }

    Scroller
    {
        id: ticker
        x: xscale(0); y: window.height - yscale(40); width: window.width; height: yscale(40)
        visible: false
    }

    Component.onCompleted:
    {
        if (showVideo)
            videoPlayer.source = settings.qmlPath + theme.backgroundVideo;
    }
}
