import QtQuick 2.0
import Base 1.0

BaseDialog
{
    id: popupMenu

    width: xscale(500)
    height: yscale(500)

    property alias model: menuList.model

    signal itemSelected(string itemText)

    function show()
    {
        menuList.setFocusedNode(0);
        popupMenu.state = "show";
    }

    function addMenuItem(path)
    {
        menuList.addNode(path);
    }

    content: Item
    {
        anchors.fill: parent

        TreeButtonList
        {
            id: menuList

            anchors.fill: parent

            focus: true
            onNodeClicked:
            {
                popupMenu.state = "";
                popupMenu.itemSelected(node.itemTitle);
            }
        }
    }
}
