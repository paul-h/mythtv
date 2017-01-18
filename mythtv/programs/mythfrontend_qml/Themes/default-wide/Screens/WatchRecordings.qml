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
                            settings.masterBackend + "Content/GetImageFile?StorageGroup=Coverart&FileName=" + Coverart
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
        x: coverImage.width + xscale(20); y: yscale(480)
        width: xscale(900); height: yscale(50)
        text: videoList.model.get(videoList.currentIndex).Title
    }
    InfoText
    {
        x: coverImage.width + xscale(220); y: yscale(530)
        width: xscale(900); height: yscale(50)
        text: videoList.model.get(videoList.currentIndex).StartTime
    }
    InfoText
    {
        x: coverImage.width + xscale(340); y: yscale(580)
        width: xscale(900); height: yscale(100)
        text: videoList.model.get(videoList.currentIndex).Description
        multiline: true
    }
}


