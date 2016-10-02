import QtQuick 2.0
import "../../Models"
import Qt.labs.folderlistmodel 2.1

Item
{
    property alias defaultFocusItem: imageList
    property alias folder: folderModel.folder

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
            width:imageList.width; height: 50

            Image
            {
                id: thumbnail
                x: 3; y:3; height: parent.height - 6; width: height
                source: fileIsDir ? "images/directory.png" : filePath
                asynchronous: true
            }
            Text
            {
                width:imageList.width; height: 50
                x: thumbnail.width + 5
                text: fileName

            }
        }
    }

    Component
    {
        id: listHighlight

        Rectangle
        {
            width:imageList.width; height: 50
            color: "green"
            opacity: 0.3
            radius: 15
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: imageList
        x: 100; y: 100; width: 1000; height: 500

        focus: true
        clip: true

        FolderListModel
        {
            id: folderModel
            folder: settings.picturePath
            nameFilters: ["*.jpg", "*.png", "*.JPG", "*.PNG"]
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
                stack.push({item: Qt.resolvedUrl("IconView.qml"), properties:{folder: model.get(currentIndex, "filePath")}});
            else
                stack.push({item: Qt.resolvedUrl("ImageView.qml"), properties:{folder: model.get(currentIndex, "filePath"), currentIndex: currentIndex, folderModel: model}});

            event.accepted = true;
            returnSound.play();
        }
    }
}


