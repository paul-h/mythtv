import QtQuick 2.0
import "../../../Models"
import Qt.labs.folderlistmodel 2.1
import Base 1.0

BaseScreen
{
    defaultFocusItem: videoList
    property alias folder: folderModel.folder

    Component.onCompleted:
    {
        showTitle(true, folderModel.folder);
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

    ButtonList
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
        highlight: ListHighlight{}

        Keys.onReturnPressed:
        {
            if (model.get(currentIndex, "fileIsDir"))
                stack.push({item: Qt.resolvedUrl("VideosGridFolder.qml"), properties:{folder: model.get(currentIndex, "filePath")}});
            else
                stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source1: "file://" + model.get(currentIndex, "filePath")}});
            event.accepted = true;
            returnSound.play();
        }
    }
}


