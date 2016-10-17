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

    Component.onCompleted:
    {
        screenBackground.showTitle = false;
        screenBackground.showTime = false;

        // try to restore the last playing station
        var url = dbUtils.getSetting("Qml_radioPlayerBookmark", hostName)

        for (var i = 0; i < radioStreamsModel.rowCount(); i++)
        {
            var itemUrl = radioStreamsModel.data(radioStreamsModel.index(i, 4));

            if (itemUrl == url)
            {
                streamList.currentIndex = i;
                channel.text = streamList.model.data(streamList.model.index(streamList.currentIndex, 1)) + streamList.model.data(streamList.model.index(streamList.currentIndex, 2));
                visualizer.source = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
                streamIcon.source = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
                break;
            }
        }

        streamPlayer.source = url;
    }

    Component.onDestruction:
    {
        screenBackground.showTitle = true;
        screenBackground.showTime = true;
        dbUtils.setSetting("Qml_radioPlayerBookmark", hostName, streamPlayer.source)
    }

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

    Audio
    {
        id: streamPlayer
        autoPlay: true

        onPositionChanged: if (trackArtistTitle == playedModel.get(0).title) playedModel.get(0).length = position - trackStart;
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(15); width: xscale(1250); height: yscale(265)
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(297); width: xscale(1250); height: yscale(170)
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(480); width: xscale(1250); height: yscale(225)
    }

    Component
    {
        id: streamRow

        Item
        {
            width:streamList.width; height: yscale(50)

            Image
            {
                id: radioIcon
                x: xscale(3); y: yscale(3); height: parent.height - yscale(6); width: height
                source: if (logourl)
                    logourl
                else
                    "images/grid_noimage.png"
            }
            LabelText
            {
                x: xscale(70); y: 0; width: xscale(350); height: yscale(46)
                text: broadcaster
            }

            LabelText
            {
                x: xscale(430); y: 0; width: xscale(370); height: yscale(46)
                text: channel
            }

            LabelText
            {
                x: xscale(810); y: 0; width: yscale(370); height: xscale(46)
                text: genre
            }
        }
    }

    Component
    {
        id: streamHighlight

        Rectangle
        {
            width:streamList.width; height: yscale(50)
            color: "green"
            opacity: 0.3
            radius: xscale(15)
            border.color: "#dd00ff00"
        }
    }

    ListView
    {
        id: streamList
        x: xscale(25); y: yscale(25); width: xscale(1225); height: yscale(250)

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
            width: playedList.width; height: yscale(50)

            Image
            {
                id: radioIcon
                x: xscale(3); y: yscale(3); height: parent.height - yscale(6); width: height
                source: if (icon)
                    icon
                else
                    "images/grid_noimage.png"
            }
            LabelText
            {
                x: xscale(70); y: 0; width: xscale(910); height: yscale(46)
                text: title
            }

//            LabelText
//            {
//                x: xscale(430); y: 0; width: xscale(370); height: yscale(46)
//                text: channel
//            }

            LabelText
            {
                x: xscale(1120); y: 0; width: xscale(100); height: yscale(46)
                text: Util.milliSecondsToString(length)
            }
        }
    }

    Component
    {
        id: playedHighlight

        Rectangle
        {
            width:playedList.width; height: yscale(50)
            color: "green"
            opacity: 0.3
            radius: xscale(15)
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
        x: xscale(25); y: yscale(307); width: xscale(1225); height: yscale(150)

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
        x: xscale(220); y: yscale(495); width: xscale(850); height: yscale(34)
        text: streamPlayer.metaData.title != undefined ? streamPlayer.metaData.title : ""
    }

    LabelText
    {
        id: trackArtist
        x: xscale(220); y: yscale(535); width: xscale(850); height: yscale(34)
        text: streamPlayer.metaData.albumTitle != undefined ? streamPlayer.metaData.albumTitle : ""
    }

    LabelText
    {
        id: channel
        x: xscale(220); y: yscale(570); width: xscale(850); height: yscale(34)
    }

    LabelText
    {
        id: urlText
        x: xscale(220); y: yscale(605); width: xscale(850); height: yscale(34)
    }

    LabelText
    {
        id: posText
        x: xscale(220); y: yscale(655); width: xscale(850); height: yscale(34)
        text: Util.milliSecondsToString(streamPlayer.position - trackStart)
    }

    Image
    {
        x: xscale(1082); y: yscale(499); width: xscale(162); height: yscale(162)
        source: "images/mm_blackhole_border.png"
    }

    Image
    {
        id: visualizer
        x: xscale(1085); y: yscale(502); width: xscale(156); height: yscale(155)
    }

    Image
    {
        x: xscale(32); y: yscale(499); width: xscale(162); height: yscale(162)
        source: "images/mm_blackhole_border.png"
    }

    Image
    {
        id: streamIcon
        x: xscale(35); y: yscale(502); width: xscale(156); height: yscale(155)
    }

    Image
    {
        id: muteIcon
        x: xscale(30); y: yscale(669)
        source: streamPlayer.muted ? "images/mm_volume_muted.png" : "images/mm_volume.png"
    }

    LabelText
    {
        id: volume
        x: xscale(70); y: yscale(660); width: xscale(156); height: yscale(35)
        text:
        {
            var volPercent = streamPlayer.volume * 100;
            Math.round(volPercent) + "%";
        }
    }

    LabelText
    {
        id: visualizerName
        x: xscale(1085); y: yscale(660); width: xscale(156); height: yscale(35)
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


