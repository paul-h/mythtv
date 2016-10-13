import QtQuick 2.0
import "base"

Item
{
    //property alias source: mediaplayer.source
    property alias defaultFocusItem: button1

    x: 0; y: 0; width: parent.width; height: parent.height

    Keys.onEscapePressed: if (stack.depth > 1) {videoplayer.stop(); stack.pop();} else Qt.quit();

    TitleText { text: "Accordion Test Page" }

    Rectangle
    {
        id: playerRect
        x: xscale(850); y: yscale(20); width: xscale(400); height: yscale(225); z:99
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
            if (event.key === Qt.Key_F)
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
                target: head;
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
            LabelText
            {
                x: xscale(20); y: 0
                fontColor: if (list.state === "selectedactive") "white"; else "purple";
                text: modelData.label
            }
            LabelText
            {
                x: xscale(500); y: 0
                text: modelData.item
            }
            LabelText
            {
                x: xscale(900); y: 0
                text: modelData.value
            }

        }
    }

    Item
    {
        id: topGroup
        x: xscale(20); y: yscale(300); width: xscale(1280 - 40); height: yscale(200)
        BaseBackground { anchors.fill: parent }
        ButtonList
        {
            id: list;
            anchors.fill: parent;
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
            KeyNavigation.right: videoplayer;

        }
    }

    LabelText { x: xscale(20); y: yscale(90); text: "SubTitle:" }
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

    BaseButton
    {
        id: button1;
        x: xscale(100); y: yscale(200);
        text: "Button Test";
        KeyNavigation.right: button2;
        KeyNavigation.left: videoplayer;
        KeyNavigation.down: list;
        onClicked: console.log("button 1 clicked");
    }

    BaseButton
    {
        id: button2;
        x: xscale(400); y: yscale(200); width: xscale(400);
        text: "Another Button";
        KeyNavigation.left: button1;
        KeyNavigation.right: videoplayer;
        KeyNavigation.down: list;
        onClicked: console.log("button 2 clicked");
    }
}
