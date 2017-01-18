import QtQuick 2.0
import QtQuick.Layouts 1.0
import QmlVlc 0.1

import "../../../Util.js" as Util


FocusScope
{
    id: root
    property alias source: mediaplayer.mrl
    property alias volume: mediaplayer.volume
    property bool loop: false
    signal playbackEnded()

    anchors.fill: parent

    Rectangle
    {
        id: background
        property alias source: mediaplayer.mrl

        color: "black"

        anchors.fill: parent

        VlcPlayer
        {
            id: mediaplayer

            onStateChanged:
            {
                if (state === VlcPlayer.Ended)
                    root.playbackEnded();
            }
        }

        VlcVideoSurface
        {
             id: videoSurface

             property bool deinterlacerEnabled: false
             source: mediaplayer;
             anchors.fill: parent;
             fillMode: VlcVideoSurface.Stretch;
             focus: true

             Keys.onReturnPressed: root.togglePaused();
             Keys.onLeftPressed: mediaplayer.time = mediaplayer.time - 30000;
             Keys.onRightPressed: mediaplayer.time = mediaplayer.time + 30000;
             Keys.onPressed:
             {
                 if (event.key === Qt.Key_I)
                 {
                     if (infoPanel.visible)
                         infoPanel.visible = false;
                     else
                         infoPanel.visible = true;
                 }
                 else if (event.key === Qt.Key_O)
                     stop();
                 else if (event.key === Qt.Key_P)
                     togglePaused();
                 else if (event.key === Qt.Key_BracketLeft)
                     changeVolume(-1.0);
                 else if (event.key === Qt.Key_BracketRight)
                     changeVolume(1.0);
                 else if (event.key === Qt.Key_M)
                     toggleMute();
                 else if (event.key === Qt.Key_D)
                     toggleInterlacer();
                 else
                     event.accepted = false;
             }
        }

        Rectangle
        {
            id: infoPanel
            x: 10; y: parent.height - 100; width: parent.width - 20; height: 90
            visible: false
            color: "black"
            opacity: 0.4
            radius: 5
            border.color: "green"
            border.width: 3

            Text
            {
                x: 10; y: 5
                id: pos
                text: "Position: " + Util.milliSecondsToString(mediaplayer.time) + " / " + Util.milliSecondsToString(mediaplayer.length)
                color: "white"
            }

            Text
            {
                x: 10; y: 20
                id: timeLeft
                text: "Remaining :" + Util.milliSecondsToString(mediaplayer.length - mediaplayer.time)
                color: "white"
            }
            Text
            {
                x: 10; y: 35
                id: vol
                text: "Volume: " + mediaplayer.volume + "%"
                color: "white"
            }

            Text
            {
                x: 500; y: 5
                id: artist
                text: if (mediaplayer.metaData && mediaplayer.metaData.albumArtist !== undefined)
                          "Artist: " + mediaplayer.metaData.albumArtist;
                      else
                          "Artist: Unknown";
                color: "white"
            }
            Text
            {
                x: 500; y: 20
                id: title
                text: if (mediaplayer.metaData && mediaplayer.metaData.title !== undefined)
                "Title: " + mediaplayer.metaData.title;
                else
                    "Title: Unknown";
                color: "white"
            }

            RowLayout
            {
                id: toolbar
                opacity: .55
                spacing: 10
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: spacing
                anchors.leftMargin: spacing * 1.5
                anchors.rightMargin: spacing * 1.5
                Behavior on anchors.bottomMargin { PropertyAnimation { duration: 250} }
                Rectangle
                {
                    height: 24
                    width: height
                    radius: width * 0.25
                    color: 'black'
                    border.width: 1
                    border.color: 'white'
                    Image
                    {
                        source: mediaplayer.playing ? mythUtils.findThemeFile("images/play.png") : mythUtils.findThemeFile("images/pause.png")
                        anchors.centerIn: parent
                    }
                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: mediaplayer.togglePause()
                    }
                }
                Rectangle
                {
                    Layout.fillWidth: true
                    height: 10
                    color: 'transparent'
                    border.width: 1
                    border.color: 'white'
                    anchors.verticalCenter: parent.verticalCenter
                    Rectangle
                    {
                        width: (parent.width - anchors.leftMargin - anchors.rightMargin) * mediaplayer.position
                        color: 'blue'
                        anchors.margins: 2
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                    }
                }
            }
        }

        MouseArea
        {
            id: playArea
            anchors.fill: parent
            onPressed: mediaplayer.play();
        }
    }

    function play()
    {
        mediaplayer.play();
    }

    function stop()
    {
        mediaplayer.stop();
    }

    function togglePaused()
    {
        if (mediaplayer.state === VlcPlayer.Paused) mediaplayer.play(); else mediaplayer.pause();
    }

    function changeVolume(amount)
    {
        if (amount < 0)
            mediaplayer.volume = Math.max(0, mediaplayer.volume + amount);
        else
            mediaplayer.volume = Math.min(100, mediaplayer.volume + amount);
    }

    function toggleMute()
    {
        mediaplayer.audio.mute = !mediaplayer.audio.mute;
    }

    function setLoopMode(doLoop)
    {
        if (doLoop)
            mediaplayer.playlist.mode = VlcPlaylist.Loop;
        else
            mediaplayer.playlist.mode = VlcPlaylist.Single;
    }

    function toggleInterlacer()
    {
        videoSurface.deinterlacerEnabled = !videoSurface.deinterlacerEnabled;

        if (videoSurface.deinterlacerEnabled)
            mediaplayer.video.deinterlace.enable("Blend")
        else
            mediaplayer.video.deinterlace.disable()
    }
}
