import QtQuick 2.0
import Base 1.0
import Dialogs 1.0

BaseScreen
{
    defaultFocusItem: button1

    Component.onCompleted:
    {
        showTitle(true, "Test Page");
        showTime(false);
        showTicker(false);
        screenBackground.muteAudio(true);
    }

    Component.onDestruction:
    {
        screenBackground.muteAudio(false);
    }

    Keys.onEscapePressed: videoplayer.stop();

    Keys.onPressed:
    {
        if (event.key === Qt.Key_M)
        {
            console.log("MENU pressed");
            popupMenu.show();
        }
    }

    Rectangle
    {
        id: playerRect
        x: xscale(850); y: yscale(20); width: xscale(400); height: yscale(225)
        color: "black"
        border.color: "white"
        border.width: xscale(4)

        property bool fullscreen: false

        VideoPlayerQmlVLC
        {
            id: videoplayer;
            anchors.margins: xscale(4)
            source: settings.hdmiEncoder

            onFocusChanged:
            {
                if (focus)
                    playerRect.border.color = "red"
                else
                    playerRect.border.color = "white"
            }
        }
        KeyNavigation.up: button2;
        KeyNavigation.down: button1;

        Keys.onPressed:
        {
            if (event.key === Qt.Key_X)
            {
                fullscreen = !fullscreen;
                if (fullscreen)
                {
                    playerRect.x = 0; playerRect.y = 0;
                    playerRect.width = parent.width; playerRect.height = parent.height;
                }
                else
                {
                    playerRect.x = 850; playerRect.y = 20;
                    playerRect.width = 400; playerRect.height = 225;
                }
            }

            if (event.key === Qt.Key_T)
                button1.enabled = !button1.enabled

            if (event.key === Qt.Key_M)
            {
                console.log("MENU pressed");
                popupMenu.show();
            }
        }

        Behavior on x
        {
            PropertyAnimation
            {
                target: playerRect;
                properties: "x";
                duration: 1000;
            }
        }
        Behavior on y
        {
            PropertyAnimation
            {
                target: playerRect;
                properties: "y";
                duration: 1000;
            }
        }
        Behavior on width
        {
            PropertyAnimation
            {
                target: playerRect;
                properties: "width";
                duration: 1000;
            }
        }
        Behavior on height
        {
            PropertyAnimation
            {
                target: playerRect;
                properties: "height";
                duration: 1000;
            }
        }
    }

    Component
    {
        id: listRow
        Item
        {
            width: parent.width; height: yscale(50)
            property bool selected: ListView.isCurrentItem
            property bool focused: list.focus

            ListBackground {}

            ListText
            {
                x: xscale(20); y: 0
                text: modelData.label
            }
            ListText
            {
                x: xscale(500); y: 0
                text: modelData.item
            }
            ListText
            {
                x: xscale(900); y: 0
                text: modelData.value
            }

        }
    }

    Item
    {
        id: topGroup
        x: xscale(20); y: yscale(260); width: xscale(1280 - 40); height: yscale(229)
        BaseBackground { anchors.fill: parent }
        ButtonList
        {
            id: list;
            spacing: 3
            anchors.fill: parent;
            anchors.margins: xscale(10)
            model:
            [
                {
                    'label': "Item1",
                    'value': "Value1",
                    'item':  "Item1"
                },
                {
                    'label': "Item2",
                    'value': "Value2",
                    'item':  "Item2"
                },
                {
                    'label': "Item3",
                    'value': "Value3",
                    'item':  "Item3"
                },
                {
                    'label': "Item4",
                    'value': "Value4",
                    'item':  "Item4"
                },
                {
                    'label': "Item5",
                    'value': "Value5",
                    'item':  "Item5"
                },
                {
                    'label': "Item6",
                    'value': "Value6",
                    'item':  "Item6"
                },
                {
                    'label': "Item7",
                    'value': "Value7",
                    'item':  "Item7"
                },
                {
                    'label': "Item8",
                    'value': "Value8",
                    'item':  "Item8"
                },
                {
                    'label': "Item9",
                    'value': "Value9",
                    'item':  "Item9"
                },
                {
                    'label': "Item10",
                    'value': "Value10",
                    'item':  "Item10"
                },
                {
                    'label': "Item11",
                    'value': "Value11",
                    'item':  "Item11"
                }
            ];
            delegate: listRow

            KeyNavigation.left: button1;
            KeyNavigation.right: accordianList;

        }
    }

    LabelText { x: xscale(20); y: yscale(90); text: "LabelText:" }
    InfoText { x: xscale(300); y: yscale(90); text: "InfoText" }
/*
    Item
    {
        id: bottomGroup
        x: xscale(20); y: yscale(510); width: xscale(1280 - 40); height: yscale(200)
        BaseBackground { anchors.fill: parent }

        Accordion
        {
            id: accordion
            anchors.fill: parent
            anchors.margins: xscale(20)

            model: [
                {
                    'label': 'Cash',
                    'value':'$4418.28',
                    'children': [
                        {
                            'label': 'Float',
                            'value': '$338.72'
                        },
                        {
                            'label': 'Cash Sales',
                            'value': '$4059.56'
                        },
                        {
                            'label': 'In/Out',
                            'value': '-$50.00',
                            'children': [
                                {
                                    'label': 'coffee/creamer',
                                    'value': '-$40.00'
                                },
                                {
                                    'label': 'staples & paper',
                                    'value': '-$10.00'
                                }

                            ]
                        }

                    ]
                },
                {
                    'label': 'Card',
                    'value': '$3314.14',
                    'children': [
                        {
                            'label': 'Debit',
                            'value': '$1204.04'
                        },
                        {
                            'label': 'Credit',
                            'value': '$2110.10'
                        }
                    ]
                }

            ]
        }

    }
*/

    Item
    {
        id: bottomGroup
        x: xscale(20); y: yscale(510); width: xscale(1280 - 40); height: yscale(200)
        
        BaseBackground 
        {
            anchors.fill: parent
        }

        AccordionList
        {
            id: accordianList
            anchors.fill: parent
            anchors.margins: xscale(10)
            focus: true

            //        KeyNavigation.left: button1;
            //        KeyNavigation.right: videoplayer;
            //        KeyNavigation.left: button1;
            //        KeyNavigation.right: videoplayer;
        }
/*
        Rectangle
        {
            id: accordianList
            anchors.fill: parent
            anchors.margins: xscale(10)

            ListModel
            {
                id: model1

                ListElement
                {
                    name: "name1"
                }
                ListElement
                {
                    name: "name2"
                }
                ListElement
                {
                    name: "name3"
                }
            }
            ListModel
            {
                id: model2
                ListElement
                {
                    name: "inside1"
                }
                ListElement
                {
                    name: "inside2"
                }
                ListElement
                {
                    name: "inside3"
                }
            }

            Component
            {
                id: delegate2

                Item
                {
                    width: 100
                    height: col2.childrenRect.height

                    Column
                    {
                        id: col2
                        anchors.left: parent.left
                        anchors.right: parent.right
                        Text
                        {
                            id: name1
                            text: name
                        }
                    }
                }
            }

            ListView
            {
                id: outer
                model: model1
                delegate: listdelegate
                anchors.fill: parent
            }

            Component
            {
                id: listdelegate

                Item
                {
                    width: 100
                    height: col.childrenRect.height

                    Column
                    {
                        id: col
                        anchors.left: parent.left
                        anchors.right: parent.right
                        Text
                        {
                            id: t1
                            text: name + "t1"
                        }
                        Text
                        {
                            id: t2
                            text: name  + "t2"
                        }
                        ListView
                        {
                            id: insidelist
                            model: model2
                            delegate: delegate2
                            contentHeight: contentItem.childrenRect.height
                            height: childrenRect.height
                            anchors.left: parent.left
                            anchors.right: parent.right
                            clip: true
                        }
                    }
                }
            }
        }
        */
    }

    BaseButton
    {
        id: button1;
        x: xscale(100); y: yscale(150); width: xscale(250);
        text: "Show Modal Dialog";
        KeyNavigation.right: button2;
        KeyNavigation.left: videoplayer;
        KeyNavigation.down: list;
        onClicked: dialog.show();
    }

    BaseButton
    {
        id: button2;
        x: xscale(400); y: yscale(100); width: xscale(400);
        text: "Another Button";
        KeyNavigation.left: button1;
        KeyNavigation.right: videoplayer;
        KeyNavigation.down: edit1;
        onClicked: console.log("button 2 clicked");
    }

    BaseEdit
    {
        id: edit1
        x: xscale(400); y: yscale(160); width: xscale(400);
        text: "Some Edit Text";
        KeyNavigation.left: button1;
        KeyNavigation.right: videoplayer;
        KeyNavigation.down: checkbox1;
        onTextHasChanged: console.log("text is now: " + text);
    }

    BaseCheckBox
    {
        id: checkbox1
        x: xscale(400); y: yscale(215)
        checked: true
        KeyNavigation.left: edit1;
        KeyNavigation.right: videoplayer;
        KeyNavigation.down: list;
        onChanged: console.log("check is now: " + checked);
    }

    OkCancelDialog
    {
        id: dialog

        title: "Dialog Test"
        message: "This is the dialog message\nClick OK to Accept this dialog. To send it away, click Cancel."
        rejectButtonText: ""
        acceptButtonText: "Save"

        width: xscale(600); height: yscale(300)

        onAccepted:
        {
            console.log("Dialog accepted signal received!");
            button1.focus = true;
        }
        onCancelled:
        {
            console.log("Dialog cancelled signal received.");
            button1.focus = true;
        }
    }
/*
    ListModel
    {
        id: menuModel

        ListElement 
        {
            itemTitle: "Apple"
            subNodes:
            [
                ListElement
                {
                    itemTitle: "Apple 1"
                },
                ListElement
                {
                    itemTitle: "Apple 2"
                },
                ListElement
                {
                    itemTitle: "Apple 3"
                },
                ListElement
                {
                    itemTitle: "Apple 4"
                },
                ListElement
                {
                    itemTitle: "Apple 5"
                },
                ListElement
                {
                    itemTitle: "Apple 6"
                }
            ]
        }
        ListElement {
            itemTitle: "Orange"
        }
        ListElement {
            itemTitle: "Banana"
        }
        ListElement {
            itemTitle: "Pear"
        }
        ListElement {
            itemTitle: "Grape"
        }
        ListElement {
            itemTitle: "Lemon"
        }
        ListElement {
            itemTitle: "Pineapple"
        }
    }
*/
    PopupMenu
    {
        id: popupMenu

        title: "Menu"
        message: "Test Options"
        width: xscale(400); height: yscale(600)

        //model: menuModel

        onItemSelected:
        {
            console.log("PopupMenu accepted signal received!: " + itemText);
            button1.focus = true;
        }
        onCancelled:
        {
            console.log("PopupMenu cancelled signal received.");
            button1.focus = true;
        }

        Component.onCompleted:
        {
            addMenuItem("Apple");
            addMenuItem("Orange");
            addMenuItem("Banana");
            addMenuItem("Pear");
            addMenuItem("Grape");
            addMenuItem("Lemon");
            addMenuItem("Pineapple");

            addMenuItem("0,Apple 1");
            addMenuItem("0,Apple 2");
            addMenuItem("0,Apple 3");
            addMenuItem("0,Apple 4");
            addMenuItem("0,Apple 5");
        }
    }
}
