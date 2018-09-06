import QtQuick 2.0
import QtQuick.Controls 1.4
import QtWebEngine 1.5
import Base 1.0
import Process 1.0
import QtAV 1.5

BaseScreen
{
    id: root
    defaultFocusItem: player
    property string command
    property var    parameters
    property string port
    property string log

    Component.onCompleted:
    {
        showTitle(false, "");
        showTime(false);
        showTicker(false);

        while (stack.busy) {};

        streamLinkProcess.start(command, parameters);
    }

    Keys.onEscapePressed:
    {
        player.stop();
        streamLinkProcess.kill();
    }

    Keys.onPressed:
    {
        event.accepted = true;

        if (event.key === Qt.Key_M)
        {

        }
        else if (event.key === Qt.Key_F1)
        {
            // RED
            player.source = "http://127.0.1.1:" + port + "/";
            player.play();
        }
        else if (event.key === Qt.Key_F2)
        {
            // GREEN
        }
        else if (event.key === Qt.Key_F3)
        {
            // YELLOW
        }
        else if (event.key === Qt.Key_F4)
        {
            //BLUE
        }
        else
        {
            event.accepted = false;
        }
    }

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
        interval: 1000; running: true; repeat: true
        onTriggered:
        {
            log = streamLinkProcess.readAll();

            if (log.includes("No playable streams found on this URL"))
                stack.pop();
            else if (log.includes("Starting server, access with one of:"))
            {
                player.source = "http://127.0.1.1:" + port + "/";
                player.play();
                player.visible = true;
            }
        }
    }
/*
    VideoPlayerQmlVLC
    {
        id: player

        visible: true;
        anchors.fill: parent

        onPlaybackEnded:
        {
            //stop();
            //stack.pop();
        }
    }
*/

    VideoPlayerQtAV
    {
        id: player

        visible: true;
        anchors.fill: parent
        fillMode: VideoOutput.Stretch

        onPlaybackEnded:
        {
            //stop();
            //stack.pop();
        }
    }
}

