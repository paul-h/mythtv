import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0
import Base 1.0

BaseScreen
{
    defaultFocusItem: treeView

    Component.onCompleted:
    {
        showTitle(true, "Test Page 2");
        showTime(false);
        showTicker(false);
        screenBackground.muteAudio(true);
/*
        treeView.addNode("Television");
        treeView.addNode("Films");

        treeView.addNode("0,Title");
        treeView.addNode("0,Genre");
        treeView.addNode("0,Year");

        treeView.addNode("1,Title");
        treeView.addNode("1,Genre");
        treeView.addNode("1,Year");

        treeView.addNode("0,0,Title 1");
        treeView.addNode("0,0,Title 2");
        treeView.addNode("0,0,Title 3");
        treeView.addNode("0,0,Title 4");
        treeView.addNode("0,0,Title 5");
        treeView.addNode("0,0,Title 6");

        treeView.addNode("0,1,Christmas");
        treeView.addNode("0,1,Comedy");
        treeView.addNode("0,1,Documentary");
        treeView.addNode("0,1,Sport");

        treeView.addNode("1,1,Christmas");
        treeView.addNode("1,1,Bond Films");
        treeView.addNode("1,1,Carry On Films");
        treeView.addNode("1,1,Comedy");
        treeView.addNode("1,1,Action");
        treeView.addNode("1,1,Sport");
*/
        //treeView.setFocusedNode("0");
    }

    ListModel
    {
        id: treeModel

        ListElement
        {
            itemTitle: "Christmas (6)"
            subNodes:
            [
                ListElement
                {
                    itemTitle: "Subitem1 title 1/1" 
                    subNodes:
                    [
                        ListElement
                        {
                            itemTitle: "Subitem2 title 1/1" 
                            subNodes:
                            [
                                ListElement
                                {
                                    itemTitle: "Subitem3 title 1"
                                },
                                ListElement
                                {
                                    itemTitle: "Subitem3 title 2" 
                                },
                                ListElement
                                {
                                    itemTitle: "Subitem3 title 3"
                                },
                                ListElement
                                {
                                    itemTitle: "Subitem3 title 4"
                                },
                                ListElement
                                {
                                    itemTitle: "Subitem3 title 5"
                                },
                                ListElement
                                {
                                    itemTitle: "Subitem3 title 6"
                                }
                            ]
                        },
                        ListElement
                        {
                            itemTitle: "Subitem2 title 2/1" 
                        },
                        ListElement
                        {
                            itemTitle: "Subitem2 title 3/1"
                        },
                        ListElement
                        {
                            itemTitle: "Subitem2 title 4/1"
                        },
                        ListElement
                        {
                            itemTitle: "Subitem2 title 5/1"
                        },
                        ListElement
                        {
                            itemTitle: "Subitem2 title 6/1"
                        }
                    ]
                },
                ListElement
                {
                    itemTitle: "Subitem1 title 2/1" 
                },
                ListElement
                {
                    itemTitle: "Subitem1 title 3/1"
                },
                ListElement
                {
                    itemTitle: "Subitem1 title 4/1"
                },
                ListElement
                {
                    itemTitle: "Subitem1 title 5/1"
                },
                ListElement
                {
                    itemTitle: "Subitem1 title 6/1"
                }
            ]
        }
        ListElement
        {
            itemTitle: "Classic Rock (2)"
            subNodes:
            [
                ListElement { itemTitle: "Subitem title 1/2" },
                ListElement { itemTitle: "Subitem title 2/2 long text long text long text long text long text long text long text" }
            ]
        }
        ListElement
        {
            itemTitle: "Pop (3)"
            subNodes:
            [
                ListElement
                {
                    itemTitle: "Pop title 1/3"
                },
                ListElement
                { 
                    itemTitle: "Pop title 2/3" 
                                        subNodes:
                    [
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 1"
                            subNodes:
                            [
                                ListElement
                                {
                                    itemTitle: "Subitem for Pop Subitem title 1"
                                }
                            ]
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 2"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 3"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 4"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 5"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 6"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 7"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 8"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 9"
                        },
                        ListElement
                        {
                            itemTitle: "Pop Subitem title 10"
                        }
                    ]

                },
                ListElement { itemTitle: "Pop title 3/3" }
            ]
        }
    }

    Component.onDestruction:
    {
        screenBackground.muteAudio(false);
    }

    BaseBackground
    {
        x: 50
        y: 100
        width: 1180
        height: 600
    }

    InfoText
    {
        id: breadCrumb
        x: 70
        y: 60
        width: 1140
    }

    TreeButtonList
    {
        id: treeView
        x: 70
        y: 120
        width: 1140
        height: 560
        columns: 4
        spacing: 10
        model: treeModel

        onNodeSelected:
        {
            breadCrumb.text = getActiveNodePath();
        }
    }
}
