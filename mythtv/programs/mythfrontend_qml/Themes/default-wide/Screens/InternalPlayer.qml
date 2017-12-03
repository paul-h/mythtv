import QtQuick 2.0
import Base 1.0
import Dialogs 1.0

BaseScreen
{
    id: root

    defaultFocusItem: player1
    property int layout: 0
    property alias source1: player1.source
    property alias title1:  player1.title
    property alias source2: player2.source
    property alias source3: player3.source
    property alias source4: player4.source

    Component.onCompleted:
    {
        showTitle(false, "");
        showTime(false);
        showTicker(false);
        showVideo(false);
        muteAudio(true);

        setLayout();
    }

    Component.onDestruction:
    {
        muteAudio(false);
    }

    onWidthChanged: setLayout();
    onHeightChanged: setLayout();

    Keys.onEscapePressed:
    {
        player1.stop();
        player2.stop();
        player3.stop();
        player4.stop();
    }

    Keys.onPressed:
    {
        if (event.key === Qt.Key_0)
        {
            layout = 0;
            setLayout();
        }
        else if (event.key === Qt.Key_1)
        {
            layout = 1;
            setLayout();
        }
        else if (event.key === Qt.Key_2)
        {
            layout = 2;
            setLayout();
        }
        else if (event.key === Qt.Key_3)
        {
            layout = 3;
            setLayout();
        }
        else if (event.key === Qt.Key_M)
        {
            popupMenu.clearMenuItems();

            popupMenu.addMenuItem("Switch Layout");
            popupMenu.addMenuItem("Source 1");

            if (root.layout > 0)
                popupMenu.addMenuItem("Source 2");

            if (root.layout > 3)
            {
                popupMenu.addMenuItem("Source 3");
                popupMenu.addMenuItem("Source 4");
            }

            popupMenu.addMenuItem("Audio");

            popupMenu.addMenuItem("0,Full Screen");
            popupMenu.addMenuItem("0,Full screen with PIP");
            popupMenu.addMenuItem("0,PBP 1/2 screen");
            popupMenu.addMenuItem("0,PBP 3/4 screen with overlap");

            popupMenu.addMenuItem("1,Freeview");
            popupMenu.addMenuItem("1,Freesat");
            popupMenu.addMenuItem("1,Sky HD");
            popupMenu.addMenuItem("1,AB IPBox");
            popupMenu.addMenuItem("1,IPBox");
            popupMenu.addMenuItem("1,ASRock PC");
            popupMenu.addMenuItem("1,Recording");
            popupMenu.addMenuItem("1,Video");

            if (root.layout > 0)
            {
                popupMenu.addMenuItem("2,Freeview");
                popupMenu.addMenuItem("2,Freesat");
                popupMenu.addMenuItem("2,Sky HD");
                popupMenu.addMenuItem("2,AB IPBox");
                popupMenu.addMenuItem("2,IPBox");
                popupMenu.addMenuItem("2,ASRock PC");
                popupMenu.addMenuItem("2,Recording");
                popupMenu.addMenuItem("2,Video");
            }

            if (root.layout > 3)
            {
                popupMenu.addMenuItem("3,Freeview");
                popupMenu.addMenuItem("3,Freesat");
                popupMenu.addMenuItem("3,Sky HD");
                popupMenu.addMenuItem("3,AB IPBox");
                popupMenu.addMenuItem("3,IPBox");
                popupMenu.addMenuItem("3,ASRock PC");
                popupMenu.addMenuItem("3,Recording");
                popupMenu.addMenuItem("3,Video");

                popupMenu.addMenuItem("4,Freeview");
                popupMenu.addMenuItem("4,Freesat");
                popupMenu.addMenuItem("4,Sky HD");
                popupMenu.addMenuItem("4,AB IPBox");
                popupMenu.addMenuItem("4,IPBox");
                popupMenu.addMenuItem("4,ASRock PC");
                popupMenu.addMenuItem("4,Recording");
                popupMenu.addMenuItem("4,Video");
            }

            popupMenu.addMenuItem("2,Source 1 Toggle Mute");

            if (root.layout > 0)
                popupMenu.addMenuItem("3,Source 2 Toggle Mute");

            if (root.layout > 3)
            {
                popupMenu.addMenuItem("5,Source 3 Toggle Mute");
                popupMenu.addMenuItem("5,Source 4 Toggle Mute");
            }

            popupMenu.show();
        }
        else
        {
            event.accepted = false;
        }
    }

    VideoPlayerQmlVLC
    {
        id: player1

         onPlaybackEnded:
         {
             stop();
             stack.pop();
         }
    }

    VideoPlayerQmlVLC
    {
        id: player2
        visible: false
    }

    VideoPlayerQmlVLC
    {
        id: player3
        visible: false
    }

    VideoPlayerQmlVLC
    {
        id: player4
        visible: false
    }

    function setLayout()
    {
        if (root.layout === 0)
        {
            // full screen
            showVideo(false);

            player1.visible = true;
            player2.visible = false;
            player3.visible = false;
            player4.visible = false;

            player1.x = 0;
            player1.y = 0;
            player1.width = root.width;
            player1.height = root.height;

            player1.play();
            player2.stop();
            player3.stop();
            player4.stop();
        }
        else if (root.layout === 1)
        {
            // fullscreen with PIP
            showVideo(false);

            player1.visible = true;
            player2.visible = true;
            player3.visible = false;
            player4.visible = false;

            player1.x = 0;
            player1.y = 0;
            player1.width = root.width;
            player1.height = root.height;

            player2.x = root.width - xscale(50) - xscale(400);
            player2.y = yscale(50);
            player2.width = xscale(400);
            player2.height = yscale(225);

            player1.play();
            player2.play();
            player3.stop();
            player4.stop();
        }
        else if (root.layout === 2)
        {
            // PBP 1/2 screen
            showVideo(true);

            player1.visible = true;
            player2.visible = true;
            player3.visible = false;
            player4.visible = false;

            player1.x = 0;
            player1.y = yscale(250);
            player1.width = root.width / 2;
            player1.height = player1.width / 1.77777777;

            player2.x = root.width / 2;
            player2.y = yscale(250);
            player2.width = root.width / 2;
            player2.height = player2.width / 1.77777777;

            player1.play();
            player2.play();
            player3.stop();
            player4.stop();
        }
        else if (root.layout === 3)
        {
            // PBP 3/4 screen with overlap
            showVideo(true);

            player1.visible = true;
            player2.visible = true;
            player3.visible = false;
            player4.visible = false;

            player1.x = 0;
            player1.y = (root.height - (((root.width / 4) * 3) / 1.77777777)) / 2;
            player1.width = (root.width / 4) * 3;
            player1.height = ((root.width / 4) * 3) / 1.77777777;

            player2.x = root.width - xscale(400);
            player2.y = (root.height - 255) / 2;
            player2.width = xscale(400);
            player2.height = yscale(225);

            player1.play();
            player2.play();
            player3.stop();
            player4.stop();
        }
    }

    PopupMenu
    {
        id: popupMenu

        title: "Menu"
        message: "LiveTV Options"

        onItemSelected:
        {
            console.log("PopupMenu accepted signal received!: " + itemText);
            player1.focus = true;

            if (itemText == "Full Screen")
            {
                root.layout = 0;
                setLayout();
            }
            else if (itemText == "Full screen with PIP")
            {
                root.layout = 1;
                setLayout();
            }
            else if (itemText == "PBP 1/2 screen")
            {
                root.layout = 2;
                setLayout();
            }
            else if (itemText == "PBP 3/4 screen with overlap")
            {
                root.layout = 3;
                setLayout();
            }
            else if (itemText == "Source 1 Toggle Mute")
                player1.toggleMute();
            else if (itemText == "Source 2 Toggle Mute")
                player2.toggleMute();
            else if (itemText == "Source 3 Toggle Mute")
                player3.toggleMute();
            else if (itemText == "Source 4 Toggle Mute")
                player4.toggleMute();
            else if (itemText == "Sky HD")
            {
                player1.source = "";
                player1.playlist.clear();
                player1.playlist.addWithOptions("v4l2:///dev/video0",
                                               [":input-slave=alsa://hw:1,0",
                                                ":v4l2-width=1280",
                                                ":v4l2-height=720",
                                                ":v4l2-aspect-ratio=16:9",
                                                ":v4l2-fps=25"
                                               ]);
                player1.playlist.play();
            }
            else if (itemText == "Freeview")
            {
                player1.source = "http://192.168.1.110:55555/BBC%20ONE%20N%20West";
            }
            else if (itemText == "Freesat")
            {
                player2.source = "http://192.168.1.110:55555/BBC%20TWO";
            }
        }

        onCancelled:
        {
            console.log("PopupMenu cancelled signal received.");
            player1.focus = true;
        }
    }
}
