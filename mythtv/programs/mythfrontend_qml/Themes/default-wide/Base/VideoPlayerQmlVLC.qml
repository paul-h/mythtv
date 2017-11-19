import QtQuick 2.0
import QtQuick.Layouts 1.0
import QmlVlc 0.1

import "../../../Util.js" as Util


FocusScope
{
    id: root
    property alias playlist: mediaplayer.playlist
    property alias source: mediaplayer.mrl
    property alias volume: mediaplayer.volume
    property bool loop: false
    signal playbackEnded()

    Rectangle
    {
        id: background
        property alias source: mediaplayer.mrl

        color: "black"

        anchors.fill: parent

        VlcPlayer
        {
            id: mediaplayer

            property bool seekable: true

            onStateChanged:
            {
                if (state === VlcPlayer.Ended)
                    root.playbackEnded();
            }

            onMediaPlayerSeekableChanged: mediaplayer.seekable = seekable

            audio.mute: false
        }

        VlcVideoSurface
        {
             id: videoSurface

             property int currentDeinterlacer: 0
             property var deinterlacers: ["None", "Blend", "Bob", "Discard", "Linear", "Mean", "X", "Yadif", "Yadif (2x)", "Phosphor", "IVTC"]

             source: mediaplayer;
             anchors.fill: parent;
             fillMode: VlcVideoSurface.Stretch;
             focus: true

             Keys.onReturnPressed: root.togglePaused();
             Keys.onLeftPressed: if (mediaplayer.seekable) mediaplayer.time = mediaplayer.time - 30000;
             Keys.onRightPressed: if (mediaplayer.seekable) mediaplayer.time = mediaplayer.time + 30000;
             Keys.onPressed:
             {
                 event.accepted = true;

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
                 else if (event.key === Qt.Key_S)
                 {
                     if (mediaplayer.mrl.indexOf("file://") == 0)
                        takeSnapshot(mediaplayer.mrl.substring(7, mediaplayer.mrl.length) + ".png");
                     else
                        takeSnapshot(settings.configPath + "snapshot.png");
                 }
                 else if (event.key === Qt.Key_BracketLeft)
                     changeVolume(-1.0);
                 else if (event.key === Qt.Key_BracketRight)
                     changeVolume(1.0);
                 else if (event.key === Qt.Key_F11)
                     toggleMute();
                 else if (event.key === Qt.Key_D)
                     toggleInterlacer();
                 else if (event.key === Qt.Key_PageUp && mediaplayer.seekable)
                     mediaplayer.time = mediaplayer.time - 600000;
                 else if (event.key === Qt.Key_PageDown && mediaplayer.seekable)
                     mediaplayer.time = mediaplayer.time + 600000;
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
                id: pos
                x: 10; y: 5
                text: "Position: " + Util.milliSecondsToString(mediaplayer.time) + " / " + Util.milliSecondsToString(mediaplayer.length)
                color: "white"
            }

            Text
            {
                id: interlacer
                x: 200; y: 5
                text: "Deinterlacer: " + videoSurface.deinterlacers[videoSurface.currentDeinterlacer]
                color: "white"
            }

            Text
            {
                id: timeLeft
                x: 10; y: 20
                text: "Remaining :" + Util.milliSecondsToString(mediaplayer.length - mediaplayer.time)
                color: "white"
            }
            Text
            {
                id: vol
                x: 10; y: 35
                text: "Volume: " + mediaplayer.volume + "%"
                color: "white"
            }

            Text
            {
                id: artist
                x: 500; y: 5
                text: if (mediaplayer.metaData && mediaplayer.metaData.albumArtist !== undefined)
                          "Artist: " + mediaplayer.metaData.albumArtist;
                      else
                          "Artist: Unknown";
                color: "white"
            }
            Text
            {
                id: title
                x: 500; y: 20
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

    function isPlaying()
    {
        return mediaplayer.state === VlcPlayer.Playing;
    }

    function play()
    {
        mediaplayer.play();
    }

    function stop()
    {
        mediaplayer.stop();
    }

    function pause()
    {
        mediaplayer.pause();
    }

    function getPaused()
    {
        if (mediaplayer.state === VlcPlayer.Paused) return true; else return false;
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

    function getMuted()
    {
        return mediaplayer.audio.mute;
    }

    function setMute(mute)
    {
        if (mute != mediaplayer.audio.mute)
            mediaplayer.audio.mute = mute;
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
        videoSurface.currentDeinterlacer++

        if (videoSurface.currentDeinterlacer >= videoSurface.deinterlacers.length)
            videoSurface.currentDeinterlacer = 0;

        if (videoSurface.currentDeinterlacer > 0)
            mediaplayer.video.deinterlace.enable(videoSurface.deinterlacers[videoSurface.currentDeinterlacer])
        else
            mediaplayer.video.deinterlace.disable()
    }

    function takeSnapshot(filename)
    {
        console.log("saving snapshot to: " + filename);
        videoSurface.grabToImage(function(result)
                                 {
                                     result.saveToFile(filename);
                                 });
    }
}
