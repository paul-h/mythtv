import QtQuick 2.0
import Base 1.0
import "../../../Models"

Item
{
    //property alias source: mediaplayer.source
    property alias defaultFocusItem: sdChannelList

    x: 0; y: 0; width: parent.width; height: parent.height

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
            width: sdChannelList.width; height: 50

            Image
            {
               id: channelImage
               x: 3; y:3; height: parent.height - 6; width: height
               source: if (icon)
                            icon
                        else
                            "images/grid_noimage.png"
            }
            Text
            {
                width:sdChannelList.width; height: 50
                x: channelImage.width + 5
                text: name + " ~ " + callsign + " ~ " + channo + " ~ " + xmltvid

            }
        }
    }

    Component
    {
        id: listHighlight

        Rectangle
        {
            width:sdChannelList.width; height: 50
            color: "green"
            opacity: 0.3
            radius: 15
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: sdChannelList
        x: 50; y: 30; width: 500; height: 500

        focus: true
        clip: true
        model: SDChannelsModel {}
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

        Keys.onReturnPressed:
        {
            returnSound.play();
        }

        KeyNavigation.left: dbChannelList;
        KeyNavigation.right: dbChannelList;
    }

    Component
    {
        id: streamRow

        Item
        {
            width:dbChannelList.width; height: 50

            Image
            {
                id: radioIcon
                x: 3; y:3; height: parent.height - 6; width: height
                source: if (icon)
                    settings.masterBackend + "Guide/GetChannelIcon?ChanId=" + chanid
                else
                    "images/grid_noimage.png"
            }

            Text
            {
                width:dbChannelList.width; height: 50
                x: radioIcon.width + 5
                text: name + " ~ " + callsign + " ~ " + channum + " ~ " + xmltvid
            }
        }
    }

    Component
    {
        id: streamHighlight

        Rectangle
        {
            width:dbChannelList.width; height: 50
            color: "green"
            opacity: 0.3
            radius: 15
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: dbChannelList
        x: 600; y: 30; width: 500; height: 500

        focus: true
        clip: true
        model: dbChannelsModel
        delegate: streamRow
        highlight: streamHighlight

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
            returnSound.play();
            var url = model.data(model.index(currentIndex, 4));
            event.accepted = true;
        }

        KeyNavigation.left: sdChannelList;
        KeyNavigation.right: saveButton;
    }

    TextEdit
    {
        id: chanNoEdit
        x: 30; y: 600
        width: 240
        height: 25
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 1))
        font.family: "Helvetica"
        font.pointSize: 20
        color: "blue"
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
        onClicked:
        {
            chanNoEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).channo;
        }
    }

    TextEdit
    {
        id: chanNameEdit
        x: 30; y: 650
        width: 240
        height: 25
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 2))
        font.family: "Helvetica"
        font.pointSize: 20
        color: "blue"
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
            chanNameEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).name;
        }
    }

    TextEdit
    {
        id: callsignEdit
        x: 400; y: 600
        width: 240
        height: 25
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 3))
        font.family: "Helvetica"
        font.pointSize: 20
        color: "blue"
        KeyNavigation.up: chanNameEdit;
        KeyNavigation.down: xmltvidEdit;
    }

    BaseButton
    {
        id: callsignButton;
        x: 650; y: 600;
        width: 50; height: 50
        text: "F3";
        KeyNavigation.right: callsignEdit
        KeyNavigation.left: xmltvidEdit
        onClicked:
        {
            callsignEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).callsign;
        }
    }

    TextEdit
    {
        id: xmltvidEdit
        x: 400; y: 650
        width: 240
        height: 25
        text: dbChannelList.model.data(dbChannelList.model.index(dbChannelList.currentIndex, 5))
        font.family: "Helvetica"
        font.pointSize: 20
        color: "blue"
        KeyNavigation.up: callsignEdit;
        KeyNavigation.down: saveButton;

    }

    BaseButton
    {
        id: xmltvButton;
        x: 650; y: 650;
        width: 50; height: 50
        text: "F4";
        KeyNavigation.right: saveButton
        KeyNavigation.left: xmltvidEdit
        onClicked:
        {
            xmltvidEdit.text = sdChannelList.model.get(sdChannelList.currentIndex).xmltvid;
        }
    }

    BaseButton
    {
        id: saveButton;
        x: xscale(900); y: yscale(630);
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
