import QtQuick 2.0
import QtQuick.XmlListModel 2.0

XmlListModel
{
    id: recordingModel

    property var busyDialog

    property var titleList: ListModel{}
    property var categoryList: ListModel{}
    property var recGroupList: ListModel{}

    source: settings.masterBackend + "Dvr/GetRecordedList"
    query: "/ProgramList/Programs/Program"
    XmlRole { name: "StartTime"; query: "xs:dateTime(StartTime)" }
    XmlRole { name: "EndTime"; query: "xs:dateTime(EndTime)" }
    XmlRole { name: "Title"; query: "Title/string()" }
    XmlRole { name: "SubTitle"; query: "SubTitle/string()" }
    XmlRole { name: "Category"; query: "Category/string()" }
    XmlRole { name: "CatType"; query: "CatType/string()" }
    XmlRole { name: "Repeat"; query: "Repeat/string()" }
    XmlRole { name: "VideoProps"; query: "VideoProps/string()" }
    XmlRole { name: "AudioProps"; query: "AudioProps/string()" }
    XmlRole { name: "SubProps"; query: "SubProps/string()" }
    XmlRole { name: "SeriesId"; query: "SeriesId/string()" }
    XmlRole { name: "ProgramId"; query: "ProgramId/string()" }
    XmlRole { name: "Stars"; query: "Stars/string()" }
    XmlRole { name: "LastModified"; query: "xs:dateTime(LastModified)" }
    XmlRole { name: "ProgramFlag"; query: "ProgramFlag/string()" }
    XmlRole { name: "Airdate"; query: "Airdate/string()" }
    XmlRole { name: "Description"; query: "Description/string()" }
    XmlRole { name: "Inetref"; query: "Inetref/string()" }
    XmlRole { name: "Season"; query: "Season/number()" }
    XmlRole { name: "Episode"; query: "Episode/number()" }
    XmlRole { name: "TotalEpisodes"; query: "TotalEpisodes/number()" }
    XmlRole { name: "FileSize"; query: "FileSize/string()" }
    XmlRole { name: "FileName"; query: "FileName/string()" }
    XmlRole { name: "HostName"; query: "HostName/string()" }
    XmlRole { name: "Coverart"; query: "Artwork/ArtworkInfos/ArtworkInfo[Type=\"coverart\"]/URL/string()" }
    XmlRole { name: "Fanart"; query: "Artwork/ArtworkInfos/ArtworkInfo[Type=\"fanart\"]/URL/string()" }
    XmlRole { name: "Banner"; query: "Artwork/ArtworkInfos/ArtworkInfo[Type=\"banner\"]/URL/string()" }
    XmlRole { name: "ChannelNo"; query: "Channel/ChanNum/string()" }
    XmlRole { name: "ChannelCallSign"; query: "Channel/CallSign/string()" }
    XmlRole { name: "ChannelName"; query: "Channel/ChannelName/string()" }
    XmlRole { name: "ChannelIcon"; query: "Channel/IconURL/string()" }
    XmlRole { name: "RecGroup"; query: "Recording/RecGroup/string()" }
    XmlRole { name: "Status"; query: "Recording/Status/string()" }
    XmlRole { name: "RecordedId"; query: "Recording/RecordedId/number()" }


    signal loaded();

    onStatusChanged:
    {
        if (status == XmlListModel.Ready)
        {
            if (busyDialog)
                busyDialog.hide();

            updateLists();

            loaded();
        }

        if (status === XmlListModel.Loading)
        {
            if (busyDialog)
                busyDialog.show();
        }

        if (status === XmlListModel.Error)
        {
            if (busyDialog)
                busyDialog.hide();
            console.log("Error: " + errorString + "\n \n \n " + recordingModel.source.toString());
        }
    }

    function updateLists()
    {
        var title;
        var category;
        var recGroup;

        var titles = [];
        var categories = [];
        var recGroups = [];

        titleList.clear();
        categoryList.clear();
        recGroupList.clear();

        for (var x = 0; x < count; x++)
        {
            title = get(x).Title;
            category = get(x).Category;
            recGroup = get(x).RecGroup;

            if (titles.indexOf(title) < 0)
                titles.push(title);

            if (categories.indexOf(category) < 0)
                categories.push(category);

            if (recGroups.indexOf(recGroup) < 0)
                recGroups.push(recGroup);
        }

        titles.sort();
        categories.sort();
        recGroups.sort();

        for (var x = 0; x < titles.length; x++)
            titleList.append({"item": titles[x]});

        for (var x = 0; x < categories.length; x++)
            categoryList.append({"item": categories[x]});

        for (var x = 0; x < recGroups.length; x++)
            recGroupList.append({"item": recGroups[x]});
    }
}
