import QtQuick 2.0
import "../../../Models"
import Qt.labs.folderlistmodel 2.1
import Base 1.0

Item
{
    //property alias source: mediaplayer.source
    property alias defaultFocusItem: videoList
    property alias folder: folderModel.folder

    x: 0; y: 0; width: parent.width; height: parent.height

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
                source: fileIsDir ? mythUtils.findThemeFile("images/directory.png") : mythUtils.findThemeFile("images/grid_noimage.png")
            }
            InfoText
            {
                width: videoList.width - coverImage.width - xscale(20); height: xscale(50)
                x: coverImage.width + xscale(20)
                text: fileName

            }
        }
    }

    Component
    {
        id: listHighlight

        Rectangle
        {
            width:videoList.width; height: 50
            color: "red"
            opacity: 0.3
            radius: 15
            border.color: "#ffff0000"
        }
    }

    ListView
    {
        id: videoList
        x: xscale(25); y: yscale(65); width: xscale(1230); height: yscale(600)

        focus: true
        clip: true

        FolderListModel {
            id: folderModel
            folder: settings.videoPath
            nameFilters: ["*.mp4", "*.flv"]
        }

        model: folderModel
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
            console.log("filename is: " + model.get(currentIndex, "filePath"));
            if (model.get(currentIndex, "fileIsDir"))
                stack.push({item: Qt.resolvedUrl("VideosGridFolder.qml"), properties:{folder: model.get(currentIndex, "filePath")}});
            else
                stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source: "file://" + model.get(currentIndex, "filePath")}});
            event.accepted = true;
            returnSound.play();
        }
    }
}


