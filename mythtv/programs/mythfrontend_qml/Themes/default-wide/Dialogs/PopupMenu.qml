import QtQuick 2.0
import Base 1.0

BaseDialog
{
    id: popupMenu

    width: xscale(400)
    height: yscale(600)

    property alias model: menuList.model

    signal itemSelected(string itemText)

    function show()
    {
        menuList.setFocusedNode(0);
        popupMenu.state = "show";
    }

    function clearMenuItems()
    {
        menuList.model.clear()
    }

    function addMenuItem(path)
    {
        menuList.addNode(path);
    }

    Keys.onPressed:
    {
        if (event.key === Qt.Key_M)
        {
            popupMenu.state = "";
            popupMenu.cancelled();
            event.accepted = true;
        }
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
