import QtQuick 2.0
import QtMultimedia 5.6
import "base"
import "../../Util.js" as Util

Item
{
    property alias defaultFocusItem: streamList
    property string trackArtistTitle: streamPlayer.metaData.title != undefined ? streamPlayer.metaData.title : ""
    property int trackStart: 0

    x: 0; y: 0; width: parent.width; height: parent.height

    onTrackArtistTitleChanged:
    {
        var title = trackArtistTitle;
        var artist = trackArtistTitle;
        var broadcaster = streamList.model.data(streamList.model.index(streamList.currentIndex, 1));
        var channel = streamList.model.data(streamList.model.index(streamList.currentIndex, 2));
        var icon = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
        if (trackArtistTitle != "")
            playedModel.insert(0, {"title": title, "artist": artist, "broadcaster": broadcaster, "channel": channel, "icon": icon, "length": 0});
        playedList.currentIndex = 0;
        trackStart = streamPlayer.position
    }

    Image
    {
        id: background
        source: themePath + "ui/background.png"
    }

    Audio
    {
        id: streamPlayer
        autoPlay: true
        source: "http://streaming.radionomy.com/JamendoLounge"

        onPositionChanged: if (trackArtistTitle == playedModel.get(0).title) playedModel.get(0).length = position - trackStart;
    }

    BaseBackground
    {
        x: 15; y: 15; width: 1250; height: 265
    }

    BaseBackground
    {
        x: 15; y: 297; width: 1250; height: 170
    }

    BaseBackground
    {
        x: 15; y: 480; width: 1250; height: 225
    }

    Component
    {
        id: streamRow

        Item
        {
            width:streamList.width; height: 50

            Image
            {
                id: radioIcon
                x: 3; y:3; height: parent.height - 6; width: height
                source: if (logourl)
                    logourl
                else
                    "images/grid_noimage.png"
            }
            LabelText
            {
                x: 70; y: 0; width: 350; height: 46
                text: broadcaster
            }

            LabelText
            {
                x: 430; y: 0; width: 370; height: 46
                text: channel
            }

            LabelText
            {
                x: 810; y: 0; width: 370; height: 46
                text: genre
            }
        }
    }

    Component
    {
        id: streamHighlight

        Rectangle
        {
            width:streamList.width; height: 50
            color: "green"
            opacity: 0.3
            radius: 15
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: streamList
        x: 25; y: 25; width: 1225; height: 250

        focus: true
        clip: true
        model: radioStreamsModel
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

        Keys.onEscapePressed: if (stack.depth > 1) {streamPlayer.stop(); stack.pop()} else Qt.quit();
        Keys.onReturnPressed:
        {
            returnSound.play();
            var url = model.data(model.index(currentIndex, 4));
            streamPlayer.source = url;
            urlText.text = url
            channel.text = streamList.model.data(streamList.model.index(streamList.currentIndex, 1)) + streamList.model.data(streamList.model.index(streamList.currentIndex, 2));
            visualizer.source = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
            streamIcon.source = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
            event.accepted = true;
        }

        KeyNavigation.left: playedList;
        KeyNavigation.right: playedList;
    }

    // played tracks list
    Component
    {
        id: playedRow

        Item
        {
            width: playedList.width; height: 50

            Image
            {
                id: radioIcon
                x: 3; y:3; height: parent.height - 6; width: height
                source: if (icon)
                    icon
                else
                    "images/grid_noimage.png"
            }
            LabelText
            {
                x: 70; y: 0; width: 910; height: 46
                text: title
            }

//            LabelText
//            {
//                x: 430; y: 0; width: 370; height: 46
//                text: channel
//            }

            LabelText
            {
                x: 1120; y: 0; width: 100; height: 46
                text: Util.milliSecondsToString(length)
            }
        }
    }

    Component
    {
        id: playedHighlight

        Rectangle
        {
            width:playedList.width; height: 50
            color: "green"
            opacity: 0.3
            radius: 15
            border.color: "#dd00ff00"
        }
    }

    ListModel
    {
        id: playedModel
    }

    ListView
    {
        id: playedList
        x: 25; y: 307; width: 1225; height: 150

        focus: true
        clip: true
        model: playedModel
        delegate: playedRow
        highlight: playedHighlight

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

        KeyNavigation.left: streamList;
        KeyNavigation.right: streamList;
    }

    // stream info panel
    LabelText
    {
        id: trackTitle
        x: 220; y: 495; width: 850; height: 34
        text: streamPlayer.metaData.title != undefined ? streamPlayer.metaData.title : ""
    }

    LabelText
    {
        id: trackArtist
        x: 220; y: 535; width: 850; height: 34
        text: streamPlayer.metaData.albumTitle != undefined ? streamPlayer.metaData.albumTitle : ""
    }

    LabelText
    {
        id: channel
        x: 220; y: 570; width: 850; height: 34
    }

    LabelText
    {
        id: urlText
        x: 220; y: 605; width: 850; height: 34
    }

    LabelText
    {
        id: posText
        x: 220; y: 655; width: 850; height: 34
        text: Util.milliSecondsToString(streamPlayer.position - trackStart)
    }

    Image
    {
        x: 1082; y:499; width: 162; height: 162
        source: "images/mm_blackhole_border.png"
    }

    Image
    {
        id: visualizer
        x: 1085; y:502; width: 156; height: 155
    }

    Image
    {
        x: 32; y:499; width: 162; height: 162
        source: "images/mm_blackhole_border.png"
    }

    Image
    {
        id: streamIcon
        x: 35; y:502; width: 156; height: 155
    }

    Image
    {
        id: muteIcon
        x: 30; y:669
        source: streamPlayer.muted ? "images/mm_volume_muted.png" : "images/mm_volume.png"
    }

    LabelText
    {
        id: volume
        x: 70; y: 660; width: 156; height: 35
        text:
        {
            var volPercent = streamPlayer.volume * 100;
            Math.round(volPercent) + "%";
        }
    }

    LabelText
    {
        id: visualizerName
        x: 1085; y: 660; width: 156; height: 35
        horizontalAlignment: Text.AlignHCenter
        text: "AlbumArt"
    }

    Keys.onPressed:
    {
        if (event.key === Qt.Key_O)
            stop();
        else if (event.key === Qt.Key_P)
            togglePaused();
        else if (event.key === Qt.Key_BracketLeft)
            changeVolume(-0.01);
        else if (event.key === Qt.Key_BracketRight)
            changeVolume(0.01);
        else if (event.key === Qt.Key_M)
            toggleMute();
    }

    Keys.onEscapePressed: if (stack.depth > 1) {streamPlayer.stop(); stack.pop()} else Qt.quit();

    function stop()
    {
        streamPlayer.stop();
    }

    function togglePaused()
    {
        if (streamPlayer.playbackState === MediaPlayer.PausedState) streamPlayer.play(); else streamPlayer.pause();
    }

    function changeVolume(amount)
    {
        if (amount < 0)
            streamPlayer.volume = Math.max(0.0, streamPlayer.volume + amount);
        else
            streamPlayer.volume = Math.min(1.0, streamPlayer.volume + amount);
    }

    function toggleMute()
    {
        streamPlayer.muted = !streamPlayer.muted;
    }
}


