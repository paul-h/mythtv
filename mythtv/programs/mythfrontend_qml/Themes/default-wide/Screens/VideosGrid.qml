import QtQuick 2.0
import "../../../Models"
import Base 1.0

Item
{
    property alias defaultFocusItem: videoList

    x: 0; y: 0; width: parent.width; height: parent.height

    Component
    {
        id: listRow

        Item
        {
            width:videoList.width; height: 50

            Image
            {
                id: coverImage
                x: xscale(3); y: yscale(3); height: parent.height - yscale(6); width: height
                source: if (Coverart)
                            settings.masterBackend + "Content/GetImageFile?StorageGroup=Coverart&FileName=" + Coverart
                        else
                            mythUtils.findThemeFile("images/grid_noimage.png")
            }
            Text
            {
                width:videoList.width; height: xscale(50)
                x: coverImage.width + xscale(5)
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
            color: "green"
            opacity: 0.3
            radius: xscale(15)
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: videoList
        x: xscale(100); y: yscale(100); width: xscale(1000); height: yscale(500)

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
        }

        Keys.onEscapePressed: if (stack.depth > 1) {stack.pop()} else Qt.quit();
        Keys.onReturnPressed:
        {
            console.log("filename is: " + model.get(currentIndex).FileName);
            stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source: settings.videoPath + model.get(currentIndex).FileName }});
            event.accepted = true;
            returnSound.play();
        }
    }
}


