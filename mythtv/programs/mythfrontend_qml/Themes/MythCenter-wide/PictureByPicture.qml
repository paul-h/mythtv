import QtQuick 2.0
import "base"
import "../../Models"

Item
{
    property alias defaultFocusItem: video1player
    property bool player1HasFocus: true

    x: 0; y: 0; width: parent.width; height: parent.height

    Keys.onEscapePressed: if (stack.depth > 1) {video1player.stop(); video2player.stop(); stack.pop();} else Qt.quit();

    Image
    {
        id: background
        anchors.fill: parent
        source: themePath + "ui/background.png"
    }

    Rectangle
    {
        id: player1Rect
        x: 0; y: yscale(20); width: xscale(640); height: yscale(357)
        color: "black"
        border.color: "white"
        border.width: xscale(4)

        VideoPlayer
        {
            id: video1player;
            anchors.margins: xscale(4)
            onFocusChanged: focusedPlayerChanged();
        }

        MouseArea
        {
            id: play1Area
            anchors.fill: parent
            onPressed: video1player.muted = !video1player.muted;
        }

        KeyNavigation.up: video2player;
        KeyNavigation.down: channelList;
    }

    Rectangle
    {
        id: player2Rect
        x: xscale(640); y: yscale(20); width: xscale(640); height: yscale(357)
        color: "black"
        border.color: "white"
        border.width: xscale(4)

        VideoPlayer
        {
            id: video2player;
            anchors.margins: xscale(4)
            onFocusChanged: focusedPlayerChanged();
        }

        MouseArea
        {
            id: play2Area
            anchors.fill: parent
            onPressed: video2player.muted = !video2player.muted;
        }

        KeyNavigation.up: video1player;
        KeyNavigation.down: channelList;
    }

    Component
    {
        id: listRow

        Item
        {
            width:channelList.width; height: yscale(50)

            Image
            {
               id: channelImage
               x: xscale(3); y: yscale(3); height: parent.height - yscale(6); width: height
               // FIXME: need to get the channel icon here
               source: "images/grid_noimage.png"
            }
            Text
            {
                width:channelList.width; height: yscale(50)
                x: channelImage.width + xscale(5)
                text: name + " ~ " + type + " ~ " + lcn + " ~ " + url

            }
        }
    }

    Component
    {
        id: listHighlight

        Rectangle
        {
            width:channelList.width; height: xscale(50)
            color: "green"
            opacity: 0.3
            radius: xscale(15)
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: channelList
        x: xscale(100); y: yscale(400); width: xscale(1000); height: yscale(300)

        focus: true
        clip: true
        model: VboxChannelsModel {}
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
            if (player1HasFocus)
                video1player.source = encodeURI(model.get(currentIndex).url);
            else
                video2player.source = encodeURI(model.get(currentIndex).url);
        }

        KeyNavigation.left: video1player;
        KeyNavigation.right: video2player;
    }

    function focusedPlayerChanged()
    {
        if (video1player.focus)
        {
            player1HasFocus = true
            player1Rect.border.color = "red"
            player2Rect.border.color = "white"
        }
        else if (video2player.focus)
        {
            player1HasFocus = false
            player1Rect.border.color = "white"
            player2Rect.border.color = "red"
        }
    }
}

