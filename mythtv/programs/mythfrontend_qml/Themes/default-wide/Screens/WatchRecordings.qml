import QtQuick 2.0
import Base 1.0
import Dialogs 1.0
import RecordingsModel 1.0

BaseScreen
{
    id: root

    property string filterTitle
    property string filterCategory
    property string filterRecGroup
    property bool dateSorterActive: true

    defaultFocusItem: recordingList

    Component.onCompleted:
    {
        showTitle(true, "Watch Recordings");
        showTime(false);
        showTicker(false);
    }

    Image
    {
        id: fanartImage
        x: xscale(0); y: yscale(0); width: xscale(1280); height: yscale(720)
    }

    InfoText
    {
        x: xscale(1050); y: yscale(5); width: xscale(200);
        text: (recordingList.currentIndex + 1) + " of " + recordingsModel.totalAvailable;
        horizontalAlignment: Text.AlignRight
    }

    BaseBackground
    {
        x: xscale(10); y: yscale(50); width: parent.width - xscale(20); height: yscale(380)
    }

    BaseBackground { x: xscale(10); y: yscale(445); width: parent.width - xscale(20); height: yscale(230) }

    Component
    {
        id: listRow

        ListItem
        {
            Image
            {
                id: coverImage
                x: xscale(13); y: yscale(3); height: parent.height - yscale(6); width: height
                source: if (Coverart)
                            settings.masterBackend + Coverart
                        else
                            mythUtils.findThemeFile("images/grid_noimage.png")
            }
            ListText
            {
                x: coverImage.width + xscale(20)
                width: xscale(670); height: yscale(50)
                text: if (Title) (SubTitle ? Title + ": " + SubTitle : Title); else ""
                fontColor: theme.labelFontColor
            }
            ListText
            {
                x: xscale(745)
                width: xscale(190); height: yscale(50)
                horizontalAlignment: Text.AlignRight
                text:
                {
                    if (Season != 0 || Episode != 0)
                        return "S:" + Season + " E:" + Episode;
                    else if (Airdate != "")
                        return Qt.formatDateTime(Airdate, "(yyyy)");
                    else
                        return ""
                }
            }
            ListText
            {
                x: coverImage.width + xscale(880)
                width: xscale(190); height: yscale(50)
                horizontalAlignment: Text.AlignRight
                text: Qt.formatDateTime(StartTime, "ddd dd/MM/yy")
            }
            ListText
            {
                x: coverImage.width + xscale(1075)
                width: xscale(80); height: yscale(50)
                horizontalAlignment: Text.AlignRight
                text: Qt.formatDateTime(StartTime, "hh:mm")
            }
            Image
            {
                id: recordingIcon
                x: xscale(1205); y: yscale(10); height: parent.height - yscale(20); width: height
                source: if (Status === "Recording") mythUtils.findThemeFile("images/record.png"); else ""
                SequentialAnimation
                {
                    running: (Status === "Recording")
                    loops: Animation.Infinite
                    NumberAnimation { target: recordingIcon; property: "opacity"; to: 0.5; duration: 1000 }
                    NumberAnimation { target: recordingIcon; property: "opacity"; to: 1.0; duration: 1000 }
                }
            }
        }
    }

    RecordingsModel
    {
        id: recordingsModel
        onTotalAvailableChanged:
        {
            recordingList.positionViewAtIndex(0, ListView.Beginning);
            recordingList.currentIndex = 0;
        }
        onDataChanged:
        {
            console.log("onDatachanged row: " + topLeft.row + ", currentIndex: " + recordingList.currentIndex);
            if (topLeft.row == recordingList.currentIndex)
                updateProgramDetails();
        }
    }

    ButtonList
    {
        id: recordingList
        x: xscale(20); y: yscale(65); width: xscale(1240); height: yscale(350)

        clip: true
        model: recordingsModel
        delegate: listRow

        Keys.onPressed:
        {
            if (event.key === Qt.Key_M)
            {
            }
            else if (event.key === Qt.Key_F1)
            {
                // RED
            }
            else if (event.key === Qt.Key_F2)
            {
                // GREEN
                filterDialog.filterTitle = root.filterTitle;
                filterDialog.filterCategory = root.filterCategory;
                filterDialog.filterRecGroup = root.filterRecGroup;
                filterDialog.show();
            }
            else if (event.key === Qt.Key_F3)
            {
                // YELLOW
                if (dateSorterActive)
                    recordingsModel.sort = "Title,Season,Episode";
                else
                    recordingsModel.sort = "StartTime";

                dateSorterActive = !dateSorterActive;

                sort.text = "Sort " + (dateSorterActive ? "(Date & Time)" : "(Season & Episode)")
            }
            else if (event.key === Qt.Key_F4)
            {
                //BLUE
            }
        }

        Keys.onReturnPressed:
        {
            var hostname = model.get(currentIndex).HostName === settings.hostName ? "localhost" : model.get(currentIndex).HostName
            var filename = "myth://" + "type=recording:server=" + hostname + ":port=6543:filename=" + model.get(currentIndex).FileName;
            stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source1: filename }});
            event.accepted = true;
            returnSound.play();
        }

        onCurrentIndexChanged: updateProgramDetails();
    }

    TitleText
    {
        id: title
        x: xscale(30); y: yscale(450)
        width: xscale(930); height: yscale(50)
    }

    Image
    {
        id: channelIcon
        x: xscale(365); y: yscale(510); width: xscale(30); height: yscale(30)
    }

    InfoText
    {
        id: channel
        x: xscale(400); y: yscale(500)
        width: xscale(900); height: yscale(50)
    }

    InfoText
    {
        id: startTime
        x: xscale(30); y: yscale(500)
        width: xscale(330); height: yscale(50)
    }

    InfoText
    {
        id: description
        x: xscale(30); y: yscale(540)
        width: xscale(900); height: yscale(100)
        multiline: true
    }

    InfoText
    {
        id: recordingStatus
        x: xscale(970); y: yscale(450)
        width: xscale(140); height: yscale(50)
        horizontalAlignment: Text.AlignRight
        fontColor: "red";
        text: "Recording"
        visible: false
    }

    InfoText
    {
        id: programCategory
        x: xscale(20); y: yscale(630); width: xscale(220)
        fontColor: "grey"
    }

    InfoText
    {
        id: programEpisode
        x: xscale(400); y: yscale(630); width: xscale(320)
        horizontalAlignment: Text.AlignHCenter
        fontColor: "grey"
    }

    InfoText
    {
        id: programFirstAired
        x: xscale(890); y: yscale(630); width: xscale(220)
        fontColor: "grey"
        horizontalAlignment: Text.AlignRight
    }

//     Image
//     {
//         id: bannerImage
//         x: xscale(300); y: yscale(480); height: yscale(60); width: 300
//         source:
//         {
//             if (recordingList.model.get(recordingList.currentIndex).Banner)
//                 settings.masterBackend + recordingList.model.get(recordingList.currentIndex).Banner
//             else
//                 ""
//         }
//     }

    Image
    {
        id: coverartImage
        x: xscale(1130); y: yscale(460); height: yscale(200); width: xscale(120)
    }

    Image
    {
        x: xscale(30); y: yscale(682); width: xscale(32); height: yscale(32)
        source: mythUtils.findThemeFile("images/red_bullet.png")
    }

    InfoText
    {
        x: xscale(65); y: yscale(682); width: xscale(250); height: yscale(32)
        text: "Options"
    }

    Image
    {
        x: xscale(350); y: yscale(682); width: xscale(32); height: yscale(32)
        source: mythUtils.findThemeFile("images/green_bullet.png")
    }

    InfoText
    {
        id: show
        x: xscale(385); y: yscale(682); width: xscale(250); height: yscale(32)
        text: "Show (All Recordings)"
    }

    Image
    {
        x: xscale(670); y: yscale(682); width: xscale(32); height: yscale(32)
        source: mythUtils.findThemeFile("images/yellow_bullet.png")
    }

    InfoText
    {
        id: sort
        x: xscale(705); y: yscale(682); width: xscale(250); height: yscale(32)
        text: "Sort (Time)"
    }

    Image
    {
        x: xscale(990); y: yscale(682); width: xscale(32); height: yscale(32)
        source: mythUtils.findThemeFile("images/blue_bullet.png")
    }

    InfoText
    {
        x: xscale(1025); y: yscale(682); width: xscale(250); height: yscale(32)
        text: "Info"
    }

    RecordingFilterDialog
    {
        id: filterDialog

        title: "Filter Recordings"
        message: ""

        model: recordingsModel

        width: xscale(600); height: yscale(500)

        onAccepted:
        {
            recordingList.focus = true;

            root.filterTitle = filterTitle;
            root.filterCategory = filterCategory;
            root.filterRecGroup = filterRecGroup;
            recordingsModel.titleRegExp = filterTitle;
            //recordingsModel.category = filterCategory;
            recordingsModel.recGroup = filterRecGroup;

            if (filterTitle == "" && filterCategory == "" && filterRecGroup == "")
                show.text = "Show (All Recordings)";
            else
                show.text = "Show (Filtered Recordings)";
        }
        onCancelled:
        {
            recordingList.focus = true;
        }
    }

    function updateProgramDetails()
    {
        // title and subtitle
        var progtitle = recordingList.model.get(recordingList.currentIndex).Title;
        var subtitle = recordingList.model.get(recordingList.currentIndex).SubTitle;
        var result = "";

        if (progtitle != undefined && progtitle.length > 0)
            result = progtitle;

        if (subtitle != undefined && subtitle.length > 0)
            result += " - " + subtitle;

        title.text = result;

        // description
        if (recordingList.model.get(recordingList.currentIndex).Description != undefined)
            description.text = recordingList.model.get(recordingList.currentIndex).Description
        else
            description.text = ""

        // recording
        recordingStatus.visible = (recordingList.model.get(recordingList.currentIndex).Status == "Recording");

        // start time
        startTime.text = Qt.formatDateTime(recordingList.model.get(recordingList.currentIndex).StartTime, "ddd dd MMM yyyy hh:mm")

        // category
        programCategory.text = recordingList.model.get(recordingList.currentIndex).Category

        // channel
        channel.text = recordingList.model.get(recordingList.currentIndex).ChanNum + " - " + recordingList.model.get(recordingList.currentIndex).CallSign + " - " + recordingList.model.get(recordingList.currentIndex).ChannelName

        // channel icon
        if (recordingList.model.get(recordingList.currentIndex).IconURL)
            channelIcon.source = settings.masterBackend + recordingList.model.get(recordingList.currentIndex).IconURL
        else
            channelIcon.source = ""

        // season and episode
        var season = recordingList.model.get(recordingList.currentIndex).Season
        var episode = recordingList.model.get(recordingList.currentIndex).Episode
        var total = recordingList.model.get(recordingList.currentIndex).TotalEpisodes
        var res = ""

        if (season > 0)
            res = "Season: " + season + " ";
        if (episode > 0)
        {
            res += " Episode: " + episode;

            if (total > 0)
                res += "/" + total;
        }

        programEpisode.text = res;

        // first aired
        if (recordingList.model.get(recordingList.currentIndex).Airdate != undefined)
            programFirstAired.text = "First Aired: " + Qt.formatDateTime(recordingList.model.get(recordingList.currentIndex).Airdate, "dd/MM/yyyy");
        else
            programFirstAired.text = ""

        // fan art
        if (recordingList.model.get(recordingList.currentIndex).Fanart)
            fanartImage.source = settings.masterBackend + recordingList.model.get(recordingList.currentIndex).Fanart
        else
            fanartImage.source =  ""

        // cover art
        if (recordingList.model.get(recordingList.currentIndex).Coverart)
            coverartImage.source = settings.masterBackend + recordingList.model.get(recordingList.currentIndex).Coverart
        else
            coverartImage.source = mythUtils.findThemeFile("images/grid_noimage.png")
    }
}
