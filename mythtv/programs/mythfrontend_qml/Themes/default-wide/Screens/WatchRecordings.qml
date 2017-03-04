import QtQuick 2.0
import "../../../Models"
import Base 1.0

BaseScreen
{
    defaultFocusItem: videoList

    Component.onCompleted:
    {
        showTitle(true, "Watch Recordings");
        showTime(false);
        showTicker(false);
        showVideo(true);
    }

    Image
    {
        id: fanartImage
        x: xscale(0); y: yscale(0); width: xscale(1280); height: yscale(720)
        source:
        {
            if (videoList.model.get(videoList.currentIndex).Fanart)
                settings.masterBackend + videoList.model.get(videoList.currentIndex).Fanart
                else
                    ""
        }
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(50); width: xscale(400); height: yscale(400)
    }

    BaseBackground
    {
        x: xscale(425); y: yscale(50); width: xscale(835); height: yscale(400)
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(465); width: xscale(1250); height: yscale(240)
    }

    Component
    {
        id: listRow

        Item
        {
            width:videoList.width; height: 50

            Image
            {
                id: coverImage
                x: xscale(13); y: yscale(3); height: parent.height - yscale(6); width: height
                source: if (Coverart)
                            settings.masterBackend + Coverart
                        else
                            mythUtils.findThemeFile("images/grid_noimage.png")
            }
            ListText
            {
                x: coverImage.width + xscale(20)
                width: xscale(450); height: yscale(50)
                text: SubTitle ? Title + ": " + SubTitle : Title
            }
            ListText
            {
                x: coverImage.width + xscale(480)
                width: xscale(190); height: yscale(50)
                horizontalAlignment: Text.AlignRight
                text: Qt.formatDateTime(StartTime, "ddd dd/MM/yy")
            }
            ListText
            {
                x: coverImage.width + xscale(680)
                width: xscale(80); height: yscale(50)
                horizontalAlignment: Text.AlignRight
                text: Qt.formatDateTime(StartTime, "hh:mm")
            }
        }
    }

    Component
    {
        id: listHighlight

        Rectangle
        {
            width:videoList.width; height: yscale(50)
            color: "red"
            opacity: 0.3
            radius: xscale(15)
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: videoList
        x: xscale(435); y: yscale(65); width: xscale(815); height: yscale(360)

        focus: true
        clip: true
        model: RecordingsModel {}
        delegate: listRow
        highlight: listHighlight

        Keys.onPressed:
        {
            if (event.key === Qt.Key_PageDown)
            {
                currentIndex = currentIndex + 6 >= model.count ? model.count - 1 : currentIndex + 6;
                event.accepted = true;
            }
            else if (event.key === Qt.Key_PageUp)
            {
                currentIndex = currentIndex - 6 < 0 ? 0 : currentIndex - 6;
                event.accepted = true;
            }
        }

        Keys.onReturnPressed:
        {
            var hostname = model.get(currentIndex).HostName === settings.hostName ? "localhost" : model.get(currentIndex).HostName
            var filename = "myth://" + hostname + "/" + model.get(currentIndex).FileName;
            console.log("Playing: " + filename);
            stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source: filename }});
            event.accepted = true;
            returnSound.play();
        }
    }

    InfoText
    {
        x: xscale(30); y: yscale(480)
        width: xscale(900); height: yscale(50)
        text: videoList.model.get(videoList.currentIndex).Title
    }

    Image
    {
        id: channelImage
        x: xscale(300); y: yscale(530); width: xscale(50); height: yscale(50)
        source:
        {
            if (videoList.model.get(videoList.currentIndex).ChannelIcon)
                settings.masterBackend + videoList.model.get(videoList.currentIndex).ChannelIcon
            else
                ""
        }
    }

    InfoText
    {
        x: xscale(400); y: yscale(530)
        width: xscale(900); height: yscale(50)
        text: videoList.model.get(videoList.currentIndex).ChannelNo + " - " + videoList.model.get(videoList.currentIndex).ChannelCallSign + " - " + videoList.model.get(videoList.currentIndex).ChannelName
    }

    InfoText
    {
        x: xscale(30); y: yscale(530)
        width: xscale(900); height: yscale(50)
        text: Qt.formatDateTime(videoList.model.get(videoList.currentIndex).StartTime, "ddd dd MMM yyyy hh:mm")
    }
    InfoText
    {
        x: xscale(30); y: yscale(580)
        width: xscale(900); height: yscale(100)
        text: videoList.model.get(videoList.currentIndex).Description
        multiline: true
    }

    Image
    {
        id: bannerImage
        x: xscale(300); y: yscale(480); height: yscale(60); width: 300
        source:
        {
            if (videoList.model.get(videoList.currentIndex).Banner)
                settings.masterBackend + videoList.model.get(videoList.currentIndex).Banner
            else
                ""
        }
    }

    Image
    {
        id: coverartImage
        x: xscale(980); y: yscale(480); height: yscale(200); width: 100
        source:
        {
            if (videoList.model.get(videoList.currentIndex).Fanart)
                settings.masterBackend + videoList.model.get(videoList.currentIndex).Coverart
                else
                    mythUtils.findThemeFile("images/grid_noimage.png")
        }
    }

}


