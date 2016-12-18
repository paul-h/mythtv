import QtQuick 2.0
import "../../../Models"
import Qt.labs.folderlistmodel 2.1

Item
{
    property alias defaultFocusItem: image
    property FolderListModel folderModel
    property alias source: image.source
    property int currentIndex

    x: 0; y: 0; width: parent.width; height: parent.height

    Image
    {
        id: image
        anchors.fill: parent
        source: folderModel.get(currentIndex, "filePath");
        asynchronous: true
        Keys.onEscapePressed: if (stack.depth > 1) {stack.pop()} else Qt.quit();
        Keys.onLeftPressed:
        {
            if (currentIndex > 0)
            {
                currentIndex -= 1;
                source = folderModel.get(currentIndex, "filePath");
            }
        }
        Keys.onRightPressed:
        {
            if (currentIndex < folderModel.count - 1)
            {
                currentIndex += 1;
                source = folderModel.get(currentIndex, "filePath");
            }
        }
    }
}


