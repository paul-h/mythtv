import QtQuick 2.0
import "../../Models"

Item
{
    property alias defaultFocusItem: videoList

    x: 0; y: 0; width: parent.width; height: parent.height

    Image
    {
        id: background
        source: themePath + "ui/background.png"
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
                x: 3; y:3; height: parent.height - 6; width: height
                source: if (Coverart)
                            settings.masterBackend + "Content/GetImageFile?StorageGroup=Coverart&FileName=" + Coverart
                        else
                            "images/grid_noimage.png"
            }
            Text
            {
                width:videoList.width; height: 50
                x: coverImage.width + 5
                text: SubTitle ? Title + ": " + SubTitle : Title

            }
        }
    }

    Component
    {
        id: listHighlight

        Rectangle
        {
            width:videoList.width; height: 50
            color: "green"
            opacity: 0.3
            radius: 15
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: videoList
        x: 100; y: 100; width: 1000; height: 500

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


