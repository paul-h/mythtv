import QtQuick 2.0
import QmlVlc 0.1
import Base 1.0
import "../../../Util.js" as Util

BaseScreen
{
    id: root

    defaultFocusItem: streamList
    property string trackArtistTitle: ""
    property int trackStart: 0

    Component.onCompleted:
    {
        showTitle(false, "");
        showTime(false);
        showTicker(false);
        muteAudio(true);
    }

    Component.onDestruction:
    {
        muteAudio(false);
    }

    onTrackArtistTitleChanged:
    {
        var a = trackArtistTitle.split(" - ");
        var title = a[0];
        var artist = a[1];
        var broadcaster = streamList.model.data(streamList.model.index(streamList.currentIndex, 1));
        var channel = streamList.model.data(streamList.model.index(streamList.currentIndex, 2));
        var icon = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
        if (trackArtistTitle != "")
            playedModel.insert(0, {"trackArtistTitle": trackArtistTitle, "title": title, "artist": artist, "broadcaster": broadcaster, "channel": channel, "icon": icon, "length": 0});
        playedList.currentIndex = 0;
        trackStart = streamPlayer.time;
        trackTitle.text = title != undefined ? title : "";
        trackArtist.text = artist != undefined ? artist : "";
    }

    VlcPlayer
    {
        id: streamPlayer

        onTimeChanged: if (trackArtistTitle != undefined && playedModel.get(0) != undefined && trackArtistTitle == playedModel.get(0).trackArtistTitle) playedModel.get(0).length = time - trackStart;

        Component.onCompleted:
        {
            // try to restore the last playing station
            var url = dbUtils.getSetting("Qml_radioPlayerBookmark", settings.hostName)

            for (var i = 0; i < radioStreamsModel.rowCount(); i++)
            {
                var itemUrl = radioStreamsModel.data(radioStreamsModel.index(i, 4));

                if (itemUrl == url)
                {
                    streamList.currentIndex = i;

                    if (streamList.model.data(streamList.model.index(streamList.currentIndex, 1)) != "")
                        channel.text = streamList.model.data(streamList.model.index(streamList.currentIndex, 1)) + " - " + streamList.model.data(streamList.model.index(streamList.currentIndex, 2));
                    else
                        channel.text = streamList.model.data(streamList.model.index(streamList.currentIndex, 2));

                    urlText.text = url;
                    visualizer.source = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
                    streamIcon.source = streamList.model.data(streamList.model.index(streamList.currentIndex, 9));
                    break;
                }
            }

            streamPlayer.mrl = url;

            var vol = dbUtils.getSetting("Qml_radioPlayerVolume", settings.hostName)
            if (vol != undefined && vol != "")
                audio.volume = vol;
            else
                audio.volume = 80
        }

        Component.onDestruction:
        {
            dbUtils.setSetting("Qml_radioPlayerBookmark", settings.hostName, mrl)
        }
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
                    mythUtils.findThemeFile("images/grid_noimage.png")
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
            streamPlayer.mrl = url;
            urlText.text = url

            if (streamList.model.data(streamList.model.index(streamList.currentIndex, 1)) != "")
                channel.text = streamList.model.data(streamList.model.index(streamList.currentIndex, 1)) + " - " + streamList.model.data(streamList.model.index(streamList.currentIndex, 2));
            else
                channel.text = streamList.model.data(streamList.model.index(streamList.currentIndex, 2));

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
                    mythUtils.findThemeFile("images/grid_noimage.png")
            }
            LabelText
            {
                x: xscale(70); y: 0; width: xscale(910); height: yscale(46)
                text: title
            }

            LabelText
            {
                x: xscale(430); y: 0; width: xscale(370); height: yscale(46)
                text: artist
            }

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
    }

    LabelText
    {
        id: trackArtist
        x: xscale(220); y: yscale(535); width: xscale(850); height: yscale(34)
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
        text: Util.milliSecondsToString(streamPlayer.time - trackStart)
    }

    Image
    {
        x: xscale(1082); y: yscale(499); width: xscale(162); height: yscale(162)
        source: mythUtils.findThemeFile("images/mm_blackhole_border.png")
    }

    Image
    {
        id: visualizer
        x: xscale(1085); y: yscale(502); width: xscale(156); height: yscale(155)
    }

    Image
    {
        x: xscale(32); y: yscale(499); width: xscale(162); height: yscale(162)
        source: mythUtils.findThemeFile("images/mm_blackhole_border.png")
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
        source: streamPlayer.audio.mute ? mythUtils.findThemeFile("images/mm_volume.png") : mythUtils.findThemeFile("images/mm_volume_muted.png")
    }

    LabelText
    {
        id: volumePercent
        x: xscale(70); y: yscale(660); width: xscale(156); height: yscale(35)
        text: streamPlayer.audio.volume + "%";
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
            changeVolume(-1.00);
        else if (event.key === Qt.Key_BracketRight)
            changeVolume(1.00);
        else if (event.key === Qt.Key_M)
            toggleMute();
        else if (event.key === Qt.Key_A)
        {
            stack.push({item: Qt.resolvedUrl("SearchRadioStreams.qml"), properties:{player: streamPlayer}});
            event.accepted = true;
            returnSound.play();
        }
    }

    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: if (trackArtistTitle != streamPlayer.mediaDescription.nowPlaying) trackArtistTitle = streamPlayer.mediaDescription.nowPlaying;
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
            streamPlayer.audio.volume = Math.max(0.0, streamPlayer.audio.volume + amount);
        else
            streamPlayer.audio.volume = Math.min(100.0, streamPlayer.audio.volume + amount);

        dbUtils.setSetting("Qml_radioPlayerVolume", settings.hostName, streamPlayer.audio.volume)
    }

    function toggleMute()
    {
        streamPlayer.audio.mute = !streamPlayer.audio.mute;
    }
}


