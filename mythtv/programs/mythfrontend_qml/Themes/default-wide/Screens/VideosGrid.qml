import QtQuick 2.0
import "../../../Models"
import Base 1.0

BaseScreen
{
    defaultFocusItem: videoList

    Component.onCompleted:
    {
        showTitle(true, "Videos Grid View");
        showTime(false);
        showTicker(false);
        showVideo(true);
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(50); width: xscale(1250); height: yscale(655)
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
            InfoText
            {
                width: videoList.width - coverImage.width - xscale(20); height: xscale(50)
                x: coverImage.width + xscale(20)
                text: SubTitle ? Title + ": " + SubTitle : Title
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
        x: xscale(25); y: yscale(65); width: xscale(1230); height: yscale(600)

        focus: true
        clip: true
        model: VideosModel {}
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
            else if (event.key === Qt.Key_E)
            {
                stack.push({item: Qt.resolvedUrl("VideoMetadataEditor.qml"), properties:{videosModel:  model, currentIndex: currentIndex}});
                event.accepted = true;
            }
        }

        Keys.onReturnPressed:
        {
            stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source: settings.videoPath + model.get(currentIndex).FileName }});
            event.accepted = true;
            returnSound.play();
        }
    }
}


