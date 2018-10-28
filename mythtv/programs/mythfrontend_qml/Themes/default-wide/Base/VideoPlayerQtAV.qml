import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtAV 1.5

import "../../../Util.js" as Util


FocusScope
{
    id: root
    //property alias playlist: mediaplayer.playlist
    property alias source: mediaplayer.source
    property alias volume: mediaplayer.volume
    property alias fillMode: mediaplayer.fillMode

    property bool loop: false
    property bool playbackStarted: false

    signal playbackEnded()
    signal showMessage(string message)

    Rectangle
    {
        id: background
        property alias source: mediaplayer.source

        color: "black"
        layer.enabled: false
        anchors.fill: parent

//        VideoFilter
//        {
//            id: vf
//            avfilter: "negate"
//        }

        Video
        {
            id: mediaplayer
            visible: (status === MediaPlayer.Buffered && playbackState === MediaPlayer.PlayingState);
            focus: true
            autoPlay: true
            backgroundColor: "black"
            anchors.fill: parent
            subtitle.engines: ["FFmpeg"]
            videoCodecPriority: ["CUDA", "FFmpeg", "VAAPI"]
            audioBackends: ["OpenAL", "Pulse", "Null"]
//            videoFilters: [vf]

            onStatusChanged:
            {
                if (status == MediaPlayer.NoMedia)
                    console.info("status: no media");
                else if (status == status == MediaPlayer.Loading)
                    console.info("status: loading");
                else if (status == MediaPlayer.Loaded)
                    console.info("status: loaded");
                else if (status == MediaPlayer.Buffering)
                    console.info("status: buffering");
                else if (status == MediaPlayer.Stalled)
                    console.info("status: Stalled");
                else if (status == MediaPlayer.Buffered)
                    console.info("status: Buffered");
                else if (status == MediaPlayer.EndOfMedia)
                    console.info("status: EndOfMedia");
                else if (status == MediaPlayer.InvalidMedia)
                    console.info("status: InvalidMedia");
                else if (status == MediaPlayer.UnknownStatus)
                    console.info("status: UnknownStatus");
            }
        }

    }

    function isPlaying()
    {
        return mediaplayer.playbackState === MediaPlayer.PlayingState;
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
        if (mediaplayer.state === MediaPlayer.PausedState) return true; else return false;
    }

    function togglePaused()
    {
        console.info("MediaPlayer.state: " + mediaplayer.state);
        if (mediaplayer.playbackState === MediaPlayer.StoppedState)
            mediaplayer.play();
        else if (mediaplayer.playbackState === MediaPlayer.PlayingState)
            mediaplayer.pause();
        else
            mediaplayer.play();
    }

    function skipBack(time)
    {
        mediaplayer.seek(mediaplayer.position - time);
    }

    function skipForward(time)
    {
        mediaplayer.seek(mediaplayer.position + time);
    }

    function changeVolume(amount)
    {
        amount = amount / 100

        if (amount < 0)
            mediaplayer.volume = Math.max(0.0, mediaplayer.volume + amount);
        else
            mediaplayer.volume = Math.min(1.0, mediaplayer.volume + amount);

        showMessage("Volume: " + Math.round(mediaplayer.volume * 100) + "%");
    }

    function getMuted()
    {
        return mediaplayer.muted;
    }

    function setMute(mute)
    {
        if (mute != mediaplayer.muted)
            mediaplayer.muted = mute;
    }

    function toggleMute()
    {
        mediaplayer.muted = !mediaplayer.muted;
    }

    function setLoopMode(doLoop)
    {
        //FIXME
    }

    function toggleInterlacer()
    {
        //FIXME

        showMessage("Deinterlacer: N/A");
    }

    function takeSnapshot(filename)
    {
        //FIXME
        console.info("saving snapshot to: " + filename);
        showMessage("Saving snapshots not currently available in this player");
    }
}

