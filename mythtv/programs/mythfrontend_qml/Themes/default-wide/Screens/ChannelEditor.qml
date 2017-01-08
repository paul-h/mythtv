import QtQuick 2.0
import Base 1.0
import "../../../Models"

Item
{
    //property alias source: mediaplayer.source
    property alias defaultFocusItem: sdChannelList

    x: 0; y: 0; width: parent.width; height: parent.height

    Component.onCompleted:
    {
        screenBackground.setTitle(false, "");
        screenBackground.showTime = false;
        screenBackground.showTicker = false;
    }

    Component.onDestruction:
    {
        screenBackground.setTitle(true, "Main Menu");
        screenBackground.showTime = true;
        screenBackground.showTicker = true;
    }

    BaseBackground { anchors.fill: parent; anchors.margins: 10 }

    Keys.onEscapePressed: if (stack.depth > 1) {stack.pop();} else Qt.quit();

    Keys.onPressed:
    {
        if (event.key === Qt.Key_F1)
        {
            chanNoEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).channo;
        }
        else if (event.key === Qt.Key_F2)
        {
            chanNameEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).name;
        }
        else if (event.key === Qt.Key_F3)
        {
            callsignEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).callsign;
        }
        else if (event.key === Qt.Key_F4)
        {
            xmltvidEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).xmltvid;
        }
        else if (event.key === Qt.Key_F5)
        {
            var chanid = dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 0))
            console.log("save button - chanid is: " + chanid);
            dbUtils.updateChannel(chanid,
                                  chanNameEdit.text,
                                  chanNoEdit.text,
                                  xmltvidEdit.text,
                                  callsignEdit.text);
        }
    }


    Component
    {
        id: listRow

        Item
        {
            width: sdChannelList.width; height: 50
            property bool selected: ListView.isCurrentItem
            property bool focused: sdChannelList.focus

            ListBackground {}

            Image
            {
               id: channelImage
               x: 3; y:3; height: parent.height - 6; width: height
               source: if (icon) icon; else mythUtils.findThemeFile("images/grid_noimage.png");
            }
            ListText
            {
                width:sdChannelList.width; height: 50
                x: channelImage.width + 5
                text: name + " ~ " + callsign + " ~ " + channo + " ~ " + xmltvid

            }
        }
    }

    ButtonList
    {
        id: sdChannelList
        x: 50; y: 30; width: 500; height: 500

        focus: true
        model: SDChannelsModel {}
        delegate: listRow
        //highlight: listHighlight

        Keys.onReturnPressed:
        {
            returnSound.play();
        }

        KeyNavigation.left:  chanNoEdit;
        KeyNavigation.right: dbChannelList;
    }

    Component
    {
        id: streamRow

        Item
        {
            width:dbChannelList.width; height: 50
            property bool selected: ListView.isCurrentItem
            property bool focused: dbChannelList.focus

            ListBackground {}

            Image
            {
                id: radioIcon
                x: 3; y:3; height: parent.height - 6; width: height
                source: if (icon)
                    settings.masterBackend + "Guide/GetChannelIcon?ChanId=" + chanid
                else
                    "images/grid_noimage.png"
            }

            ListText
            {
                width: dbChannelList.width; height: 50
                x: radioIcon.width + 5
                text: name + " ~ " + callsign + " ~ " + channum + " ~ " + xmltvid
            }
        }
    }


    ButtonList
    {
        id: dbChannelList
        x: 600; y: 30; width: 500; height: 500

        focus: true
        model: dbChannelsModel
        delegate: streamRow

        Keys.onEscapePressed: if (stack.depth > 1) {stack.pop()} else Qt.quit();
        Keys.onReturnPressed:
        {
            returnSound.play();
            var url = model.data(model.index(currentIndex, 4));
            event.accepted = true;
        }

        KeyNavigation.left: sdChannelList;
        KeyNavigation.right: saveButton;
    }

    BaseEdit
    {
        id: chanNoEdit
        x: 30; y: 600
        width: 240
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 1))
        KeyNavigation.up: sdChannelList
        KeyNavigation.right: chanNoButton
        KeyNavigation.down: chanNameEdit
    }

    BaseButton
    {
        id: chanNoButton;
        x: 280; y: 600;
        width: 50; height: 50
        text: "F1";
        KeyNavigation.right: chanNameEdit
        KeyNavigation.left: chanNoEdit
        KeyNavigation.down: chanNameEdit

        onClicked:
        {
            //chanNoEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).channo;
            dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 1)) = sdChannelList.model.get(sdChannelList.currentIndex).channo;
        }
    }

   BaseEdit
    {
        id: chanNameEdit
        x: 30; y: 650
        width: 240
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 2))
        KeyNavigation.up: chanNoEdit;
        KeyNavigation.right: chanNameButton
        KeyNavigation.down: callsignEdit;
    }

    BaseButton
    {
        id: chanNameButton;
        x: 280; y: 650;
        width: 50; height: 50
        text: "F2";
        KeyNavigation.right: chanNameEdit
        KeyNavigation.left: callsignEdit
        onClicked:
        {
            //chanNameEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).name;
            dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 2)) = sdChannelList.model.get(sdChannelList.currentIndex).name
        }
    }

    BaseEdit
    {
        id: callsignEdit
        x: 400; y: 600
        width: 390
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 3))
        KeyNavigation.up: chanNameEdit;
        KeyNavigation.down: xmltvidEdit;
    }

    BaseButton
    {
        id: callsignButton;
        x: 800; y: 600;
        width: 50; height: 50
        text: "F3";
        KeyNavigation.right: callsignEdit
        KeyNavigation.left: xmltvidEdit
        onClicked:
        {
            //callsignEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).callsign;
            dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 3)) = sdChannelList.model.get(sdChannelList.currentIndex).callsign;
        }
    }

    BaseEdit
    {
        id: xmltvidEdit
        x: 400; y: 650
        width: 390
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 5))
        KeyNavigation.up: callsignEdit;
        KeyNavigation.down: saveButton;
    }

    BaseButton
    {
        id: xmltvButton;
        x: 800; y: 650;
        width: 50; height: 50
        text: "F4";
        KeyNavigation.right: saveButton
        KeyNavigation.left: xmltvidEdit
        onClicked:
        {
            //xmltvidEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).xmltvid;
            dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 5)) = sdChannelList.model.get(sdChannelList.currentIndex).xmltvid;
        }
    }

    BaseButton
    {
        id: saveButton;
        x: xscale(1050); y: yscale(630);
        text: "Save";
        KeyNavigation.right: chanNoEdit
        KeyNavigation.left: xmltvidEdit
        KeyNavigation.down: dbChannelList
        onClicked:
        {
            var chanid = dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 0))
            console.log("save button - chanid is: " + chanid);
            dbUtils.updateChannel(chanid,
                          chanNameEdit.text,
                          chanNoEdit.text,
                          xmltvidEdit.text,
                          callsignEdit.text);
        }
    }
}
