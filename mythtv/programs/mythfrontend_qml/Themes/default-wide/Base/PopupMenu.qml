import QtQuick 2.0

FocusScope
{
    id: popupMenu

    width: 500
    height: 500
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    property string title: ""
    property string message: ""

    property alias model: menuList.model

    signal accepted(string itemText)
    signal cancelled

    function show()
    {
        menuList.setFocusedNode(0);
        popupMenu.state = "show";
    }

    function addMenuItem(path)
    {
        menuList.addNode(path);
    }

    Keys.onEscapePressed:
    {
        popupMenu.state = "";
        popupMenu.cancelled();
    }

    Item
    {
        id: dialog

        opacity: 0
        anchors.fill: parent

        BaseDialogBackground
        {
            id: background
            anchors.fill: parent
        }

        Column
        {
            id: column
            width: parent.width
            height: 500

            spacing: 10

            TitleText
            {
                id: titleText
                x: 20; y: 5
                width: parent.width - 30
                text: title
                horizontalAlignment: Text.AlignHCenter
            }

            InfoText
            {
                id: messageText
                x: 20; y: 5
                width: parent.width - 40
                height: 50
                text: message
                multiline: true
                horizontalAlignment: Text.AlignHCenter
            }


            TreeButtonList
            {
                id: menuList

                x: 20; y: 5
                width: parent.width - 40
                height: 400
                focus: true
                onNodeClicked:
                {
                    popupMenu.state = "";
                    popupMenu.accepted(node.itemTitle);
                }
            }
        }
    }

    states:
    [
        State
        {
            name: ""
            PropertyChanges
            {
                target: dialog
                opacity: 0
            }
            PropertyChanges
            {
                target: popupMenu
                focus: false
            }
        },
        State
        {
            name: "show"
            PropertyChanges
            {
                target: dialog
                opacity: 1
            }
            PropertyChanges
            {
                target: popupMenu
                focus: true
            }
        }
    ]

    transitions:
    [
        Transition
        {
            from: ""
            to: "show"
            SequentialAnimation
            {
                NumberAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: 750 }
            }
        },
        Transition
        {
            from: "show"
            to: ""
            NumberAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: 750 }
        }
    ]
}
