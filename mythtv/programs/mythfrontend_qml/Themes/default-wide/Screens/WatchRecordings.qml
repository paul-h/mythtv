import QtQuick 2.0
import "../../../Models"
import Base 1.0
import Dialogs 1.0
import SortFilterProxyModel 0.2

BaseScreen
{
    id: root

    property string filterTitle
    property string filterCategory
    property string filterRecGroup

    defaultFocusItem: videoList

    Component.onCompleted:
    {
        showTitle(true, "Watch Recordings");
        showTime(false);
        showTicker(false);
    }

    property bool dateSorterActive: true;

    property list<QtObject> dateSorter:
    [
        RoleSorter { roleName: "StartTime"; ascendingOrder: false}
    ]

    property list<QtObject> episodeSorter:
    [
        RoleSorter { roleName: "Title" },
        RoleSorter { roleName: "Season" },
        RoleSorter { roleName: "Episode" }
    ]

    SortFilterProxyModel
    {
        id: recordingsProxyModel
        sourceModel: RecordingsModel {}
        filters:
        [
            AllOf
            {
                RegExpFilter
                {
                    roleName: "Title"
                    pattern: filterTitle
                    caseSensitivity: Qt.CaseInsensitive
                }
                RegExpFilter
                {
                    roleName: "Category"
                    pattern: filterCategory
                    caseSensitivity: Qt.CaseInsensitive
                }
                RegExpFilter
                {
                    roleName: "RecGroup"
                    pattern: filterRecGroup
                    caseSensitivity: Qt.CaseInsensitive
                }
            }
        ]
        sorters: dateSorter
    }

    Image
    {
        id: fanartImage
        x: xscale(0); y: yscale(0); width: xscale(1280); height: yscale(720)
        source:
        {
            if (videoList.model.get(videoList.currentIndex).Fanart)
                settings.masterBackend + videoList.model.get(videoList.currentIndex).Fanart
                else
                    ""
        }
    }

    InfoText
    {
        x: xscale(1050); y: yscale(5); width: xscale(200);
        text: (videoList.currentIndex + 1) + " of " + recordingsProxyModel.count;
        horizontalAlignment: Text.AlignRight
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(50); width: xscale(400); height: yscale(400)
    }

    BaseBackground
    {
        x: xscale(425); y: yscale(50); width: xscale(835); height: yscale(400)
    }

    BaseBackground
    {
        x: xscale(15); y: yscale(465); width: xscale(1250); height: yscale(240)
    }

    Component
    {
        id: listRow

        Item
        {
            width: parent.width; height: yscale(50)
            property bool selected: ListView.isCurrentItem
            property bool focused: videoList.focus

            //ListBackground {}

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
                width: xscale(450); height: yscale(50)
                text: SubTitle ? Title + ": " + SubTitle : Title
                fontColor: theme.labelFontColor
            }
            ListText
            {
                x: coverImage.width + xscale(480)
                width: xscale(190); height: yscale(50)
                horizontalAlignment: Text.AlignRight
                text: Qt.formatDateTime(StartTime, "ddd dd/MM/yy")
            }
            ListText
            {
                x: coverImage.width + xscale(680)
                width: xscale(80); height: yscale(50)
                horizontalAlignment: Text.AlignRight
                text: Qt.formatDateTime(StartTime, "hh:mm")
            }
        }
    }

    ListView
    {
        id: videoList
        x: xscale(435); y: yscale(65); width: xscale(815); height: yscale(360)

        focus: true
        clip: true
        model: recordingsProxyModel
        delegate: listRow
        highlight: ListHighlight {}

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
            else if (event.key === Qt.Key_M)
            {
                filterDialog.filterTitle = root.filterTitle;
                filterDialog.filterCategory = root.filterCategory;
                filterDialog.filterRecGroup = root.filterRecGroup;
                filterDialog.show();
            }
            else if (event.key === Qt.Key_S)
            {
                if (dateSorterActive)
                    recordingsProxyModel.sorters = episodeSorter;
                else
                    recordingsProxyModel.sorters = dateSorter

                dateSorterActive = !dateSorterActive;
            }
        }

        Keys.onReturnPressed:
        {
            var hostname = model.get(currentIndex).HostName === settings.hostName ? "localhost" : model.get(currentIndex).HostName
            var filename = "myth://" + hostname + "/" + model.get(currentIndex).FileName;
            console.log("Playing: " + filename);
            stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source1: filename }});
            event.accepted = true;
            returnSound.play();
        }
    }

    InfoText
    {
        x: xscale(30); y: yscale(480)
        width: xscale(900); height: yscale(50)
        text:
        {
            var title = videoList.model.get(videoList.currentIndex).Title;
            var subtitle = videoList.model.get(videoList.currentIndex).SubTitle;
            var season = videoList.model.get(videoList.currentIndex).Season;
            var episode = videoList.model.get(videoList.currentIndex).Episode;
            var total = videoList.model.get(videoList.currentIndex).TotalEpisodes;
            var result = "";

            if (title != undefined && title.length > 0)
                result = title;

            if (subtitle != undefined && subtitle.length > 0)
                result += " - " + subtitle;

            if (season > 0 && episode > 0)
            {
                result += " (s:" + season + " e:" + episode

                if (total > 0)
                    result += " of " + total;

                result += ")";
            }

            return result;
        }
    }

    Image
    {
        id: channelImage
        x: xscale(300); y: yscale(530); width: xscale(50); height: yscale(50)
        source:
        {
            if (videoList.model.get(videoList.currentIndex).ChannelIcon)
                settings.masterBackend + videoList.model.get(videoList.currentIndex).ChannelIcon
            else
                ""
        }
    }

    InfoText
    {
        x: xscale(400); y: yscale(530)
        width: xscale(900); height: yscale(50)
        text: videoList.model.get(videoList.currentIndex).ChannelNo + " - " + videoList.model.get(videoList.currentIndex).ChannelCallSign + " - " + videoList.model.get(videoList.currentIndex).ChannelName
    }

    InfoText
    {
        x: xscale(30); y: yscale(530)
        width: xscale(900); height: yscale(50)
        text: Qt.formatDateTime(videoList.model.get(videoList.currentIndex).StartTime, "ddd dd MMM yyyy hh:mm")
    }
    InfoText
    {
        x: xscale(30); y: yscale(580)
        width: xscale(900); height: yscale(100)
        text:
        {
            if (videoList.model.get(videoList.currentIndex).Description != undefined)
                videoList.model.get(videoList.currentIndex).Description
            else
                ""
        }
        multiline: true
    }

//     Image
//     {
//         id: bannerImage
//         x: xscale(300); y: yscale(480); height: yscale(60); width: 300
//         source:
//         {
//             if (videoList.model.get(videoList.currentIndex).Banner)
//                 settings.masterBackend + videoList.model.get(videoList.currentIndex).Banner
//             else
//                 ""
//         }
//     }

    Image
    {
        id: coverartImage
        x: xscale(980); y: yscale(480); height: yscale(200); width: 100
        source:
        {
            if (videoList.model.get(videoList.currentIndex).Fanart)
                settings.masterBackend + videoList.model.get(videoList.currentIndex).Coverart
                else
                    mythUtils.findThemeFile("images/grid_noimage.png")
        }
    }

    RecordingFilterDialog
    {
        id: filterDialog

        title: "Filter Recordings"
        message: ""

        recordingsModel: recordingsProxyModel.sourceModel

        width: 600; height: 500

        onAccepted:
        {
            videoList.focus = true;

            root.filterTitle = filterTitle;
            root.filterCategory = filterCategory;
            root.filterRecGroup = filterRecGroup;
        }
        onCancelled:
        {
            videoList.focus = true;
        }
    }
}


