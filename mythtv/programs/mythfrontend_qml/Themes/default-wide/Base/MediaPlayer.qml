import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtWebEngine 1.3
import Base 1.0
import Process 1.0
import QtAV 1.5
import QtQuick.XmlListModel 2.0

import "../../../Util.js" as Util

FocusScope
{
    id: root

    // channel list
    property var feedList;
    property int currentFeed: 0

    property string streamlinkPort: "4545"
    property string log

    // one of VLC, FFMPEG, Browser, YouTube, RailCam, StreamLink, Internal
    property string player: ""

    property bool showBorder: true

    Component.onCompleted:
    {
        startPlayback();
    }

    Component.onDestruction: showMouse(false)

    Process
    {
        id: streamLinkProcess
        onFinished:
        {
            if (exitStatus == Process.NormalExit)
            {
                //stack.pop();
            }
        }
    }

    Timer
    {
        id: checkProcessTimer
        interval: 1000; running: false; repeat: true
        onTriggered:
        {
            log = streamLinkProcess.readAll();

            if (log.includes("No playable streams found on this URL"))
            {
                //TODO show error message here
                console.log("No playable streams found on this URL");
            }
            else if (log.includes("Starting server, access with one of:"))
            {
                qtAVPlayer.visible = true;
                switchURL("http://127.0.1.1:" + streamlinkPort + "/");
            }
        }
    }

    Rectangle
    {
        id: playerRect
        anchors.fill: parent
        focus: true
        color: "black"
        border.width: root.showBorder ? 5 : 0
        border.color: root.focus ? "green" : "white"
    }

    WebEngineView
    {
        id: webPlayer
        visible: false
        enabled: visible
        anchors.fill: parent
        anchors.margins: playerRect.border.width
        url: ""
        settings.pluginsEnabled: true
        settings.javascriptEnabled: true
    }

    WebEngineView
    {
        id: railcamBrowser
        x: 0
        y: 0
        width: 10;
        height: 10;

        zoomFactor: parent.width / xscale(1280)
        visible: false
        enabled: visible

        settings.pluginsEnabled: true

        onLoadingChanged:
        {
            if (loadRequest.status == WebEngineLoadRequest.LoadSucceededStatus)
            {
                x = (parent.width - contentsSize.width) / 2;
                y = parent.height - contentsSize.height - yscale(20);
                width = contentsSize.width;
                height = contentsSize.height;
            }
        }
    }

    VideoPlayerQmlVLC
    {
        id: vlcPlayer

        visible: false
        enabled: visible
        anchors.fill: parent
        anchors.margins: 5

        onPlaybackEnded:
        {
            //stop();
            //stack.pop();
        }

        onShowMessage:
        {
            root.showMessage(message);
        }
    }

    VideoPlayerQtAV
    {
        id: qtAVPlayer

        visible: false
        enabled: visible
        anchors.fill: parent
        anchors.margins: 5

        fillMode: VideoOutput.Stretch

        onPlaybackEnded:
        {
            //stop();
            //stack.pop();
        }

        onShowMessage:
        {
            console.log("Got show message signal");
            root.showMessage(message);
        }
    }

    Timer
    {
        id: infoTimer
        interval: 6000; running: false; repeat: false
        onTriggered: infoPanel.visible = false;
    }

    BaseBackground
    {
        id: infoPanel
        x: xscale(10); y: parent.height - yscale(170); width: parent.width - xscale(20); height: yscale(110)
        visible: false

        TitleText
        {
            id: title
            x: xscale(10); y: yscale(5); width: parent.width - xscale(20)
            text:
            {
                if (root.feedList.get(root.currentFeed).title != "")
                    return root.feedList.get(root.currentFeed).title
                else
                    return root.feedList.get(root.currentFeed).url
            }

            verticalAlignment: Text.AlignTop
        }

        InfoText
        {
            id: pos
            x: xscale(50); y: yscale(45)
            text:
            {
                if (getActivePlayer() === "VLC")
                    return "Position: " + Util.milliSecondsToString(vlcPlayer.getPosition()) + " / " + Util.milliSecondsToString(vlcPlayer.getDuration())
                else
                    return "Position: " + "N/A"
            }
        }

        InfoText
        {
            id: timeLeft
            x: parent.width - width - xscale(15); y: yscale(45)
            text:
            {
                if (getActivePlayer() === "VLC")
                    return Util.milliSecondsToString(vlcPlayer.getDuration() - vlcPlayer.getPosition())
                else
                    "Remaining :" + "N/A"
            }

            horizontalAlignment: Text.AlignRight
        }

        RowLayout
        {
            id: toolbar
            opacity: .55
            spacing: xscale(10)
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: spacing
            anchors.leftMargin: spacing * xscale(1.5)
            anchors.rightMargin: spacing * xscale(1.5)
            Behavior on anchors.bottomMargin { PropertyAnimation { duration: 250} }
            Rectangle
            {
                height: yscale(24)
                width: height
                radius: width * xscale(0.25)
                color: 'black'
                border.width: xscale(1)
                border.color: 'white'
                Image
                {
                    source: mythUtils.findThemeFile("images/play.png") //mediaplayer.playing ? mythUtils.findThemeFile("images/play.png") : mythUtils.findThemeFile("images/pause.png")
                    anchors.centerIn: parent
                }
            }
            Rectangle
            {
                Layout.fillWidth: true
                height: yscale(10)
                color: 'transparent'
                border.width: xscale(1)
                border.color: 'white'
                anchors.verticalCenter: parent.verticalCenter
                Rectangle
                {
                    width:
                    {
                        var position = 1;

                        if (getActivePlayer() === "VLC")
                            position = vlcPlayer.getPosition() / vlcPlayer.getDuration();

                        return (parent.width - anchors.leftMargin - anchors.rightMargin) * position;
                    }
                    color: 'blue'
                    anchors.margins: xscale(2)
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                }
            }
        }
    }

    Timer
    {
        id: messageTimer
        interval: 6000; running: false; repeat: false
        onTriggered: messagePanel.visible = false;
    }

    BaseBackground
    {
        id: messagePanel
        x: xscale(100); y: yscale(120); width: xscale(400); height: yscale(110)
        visible: false

        InfoText
        {
            id: messageText
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
        }
    }

    function showMessage(message)
    {
        messageText.text = message;
        messagePanel.visible = true;
        messageTimer.restart();
    }

    function getActivePlayer()
    {
        if (webPlayer.visible === true)
            return "BROWSER";
        else if (vlcPlayer.visible === true)
            return "VLC";
        else if (qtAVPlayer.visible === true)
            return "QTAV";
        else
            return "NONE";
    }

    function startPlayback()
    {
        if (root.feedList == undefined)
            return;

        var newPlayer = root.feedList.get(root.currentFeed).player;
        switchPlayer(newPlayer);
        root.player = newPlayer;

        var newURL = root.feedList.get(root.currentFeed).url;
        switchURL(newURL);
    }

    function switchPlayer(newPlayer)
    {
        // we always need to restart the StreamLink process even if it is already running
        if (newPlayer === "StreamLink")
        {
            webPlayer.visible = false;
            vlcPlayer.visible = false;
            qtAVPlayer.visible = false;
            showMouse(false);

            var url = root.feedList.get(root.currentFeed).url;
            var command = "streamlink"
            var parameters = ["--player-external-http", "--player-external-http-port", streamlinkPort, url, "best"]
            streamLinkProcess.start(command, parameters);

            checkProcessTimer.running = true;

            root.player = newPlayer;
            return;
        }

        if (newPlayer === root.player)
            return;

        streamLinkProcess.kill();
        checkProcessTimer.running = false;

        if (newPlayer === "VLC" || newPlayer === "Internal")
        {
            webPlayer.visible = false;
            vlcPlayer.visible = true;
            qtAVPlayer.visible = false;
            showMouse(false);
        }
        else if (newPlayer === "FFMPEG")
        {
            webPlayer.visible = false;
            vlcPlayer.visible = false;
            qtAVPlayer.visible = true;
            showMouse(false);
        }
        else if (newPlayer === "WebBrowser" || newPlayer === "RailCam" || newPlayer === "YouTube")
        {
            webPlayer.visible = true;
            vlcPlayer.visible = false;
            qtAVPlayer.visible = false;
            showMouse(true);
        }
        else
        {
            console.log("Got unknown player '" + newPlayer + "' in MediaPlayer!");
            return;
        }

        root.player = newPlayer;
    }

    function switchURL(newURL)
    {
        railcamBrowser.visible = false;
        railcamBrowser.url = "";

        stop();

        if (root.player === "VLC" || root.player === "Internal")
        {
            vlcPlayer.source = newURL;
            //qtAVPlayer.source = "";
            //webPlayer.url = ""
        }
        else if (root.player === "FFMPEG")
        {
            //vlcPlayer.source = ""
            qtAVPlayer.source = newURL;
            //webPlayer.url = ""
        }
        else if (root.player === "WebBrowser" || root.player === "RailCam")
        {
            //vlcPlayer.source = ""
            //qtAVPlayer.source = "";
            webPlayer.url = newURL;
        }
        else if (root.player === "YouTube")
        {
            //vlcPlayer.source = ""
            //qtAVPlayer.source = "";
            webPlayer.url = newURL;
        }
        else if (root.player === "StreamLink")
        {
            //vlcPlayer.source = "";
            qtAVPlayer.source = newURL;
            //webPlayer.url = ""
        }
        else
        {
            console.log("Unknown player '" + root.player + "' in MediaPlayer!");
            return;
        }
    }

    function play()
    {
        startPlayback();
    }

    function stop()
    {
        if (getActivePlayer() === "VLC")
        {
            vlcPlayer.stop();
            vlcPlayer.source = "";
        }
        else if (getActivePlayer() === "QTAV")
        {
            qtAVPlayer.stop();
            qtAVPlayer.source = "";
        }
        else if (getActivePlayer() === "BROWSER")
            webPlayer.url = "";
    }

    function togglePaused()
    {
        if (getActivePlayer() === "VLC")
            vlcPlayer.togglePaused();
        else if (getActivePlayer() === "QTAV")
            qtAVPlayer.togglePaused();
    }

    function toggleMute()
    {
        if (getActivePlayer() === "VLC")
        {
            vlcPlayer.toggleMute();
            showMessage("Mute: " + (vlcPlayer.getMuted() ? "On" : "Off"));
        }
        else if (getActivePlayer() === "QTAV")
        {
            qtAVPlayer.toggleMute();
            showMessage("Mute: " + (qtAVPlayer.getMuted() ? "On" : "Off"));
        }
        else if (getActivePlayer() === "BROWSER")
        {
            webPlayer.audioMuted = !webPlayer.audioMuted;
            webPlayer.triggerWebAction(WebEngineView.ToggleMediaMute);
            showMessage("Mute: " + (webPlayer.audioMuted ? "On" : "Off"));
        }
    }

    function changeVolume(amount)
    {
        if (getActivePlayer() === "VLC")
            vlcPlayer.changeVolume(amount);
        else if (getActivePlayer() === "QTAV")
            qtAVPlayer.changeVolume(amount);
    }

    function skipBack(time)
    {
        if (getActivePlayer() === "VLC")
            vlcPlayer.skipBack(time);
        else if (getActivePlayer() === "QTAV")
            qtAVPlayer.skipBack(time);
    }

    function skipForward(time)
    {
        if (getActivePlayer() === "VLC")
            vlcPlayer.skipForward(time);
        else if (getActivePlayer() === "QTAV")
            qtAVPlayer.skipForward(time);
    }

    function toggleInterlacer()
    {
        if (getActivePlayer() === "VLC")
            vlcPlayer.toggleInterlacer();
        else if (getActivePlayer() === "QTAV")
            qtAVPlayer.toggleInterlacer();
    }

    function takeSnapshot()
    {
        var filename;

        if (root.feedList.get(root.currentFeed).url.indexOf("file://") == 0)
            filename = root.feedList.get(root.currentFeed).url.substring(7, root.feedList.get(root.currentFeed).url.length) + ".png";
        else
            filename = settings.configPath + "snapshot.png";

        if (getActivePlayer() === "VLC")
            vlcPlayer.takeSnapshot(filename)
        else
            console.log("Media Player '" + root.player + "' does not support taking snapshots");
    }

    function showInfo(restart)
    {
        if (restart)
        {
            // restart the timer
            infoPanel.visible = true;
            infoTimer.restart();
        }
        else
        {
            if (infoPanel.visible)
                infoPanel.visible = false;
            else
            {
                infoPanel.visible = true;
                infoTimer.start();
            }
        }
    }

    function showRailCamDiagram()
    {
        if (root.feedList.get(root.currentFeed).player === "RailCam")
        {
            if (railcamBrowser.visible)
            {
                railcamBrowser.url = "";
                railcamBrowser.visible = false;
            }
            else
            {
                railcamBrowser.zoomFactor = xscale(root.feedList.get(root.currentFeed).zoom)
                railcamBrowser.url = root.feedList.get(root.currentFeed).website;
                railcamBrowser.visible = true;
            }
        }
    }

    function nextFeed()
    {
        if (currentFeed == feedList.count - 1)
        {
            currentFeed = 0;
        }
        else
            currentFeed++;

        startPlayback();

        showInfo(true);
    }

    function previousFeed()
    {
        if (currentFeed == 0)
        {
            currentFeed = feedList.count - 1;
        }
        else
            currentFeed--;

        startPlayback();

        showInfo(true);
    }
}

