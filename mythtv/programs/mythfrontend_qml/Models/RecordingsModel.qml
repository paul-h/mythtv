import QtQuick 2.0
import QtQuick.XmlListModel 2.0

XmlListModel
{
    id: recordingModel

    property string title: ""
    property string recGroup: "Films"
    property string storageGroup:""
    property int    count: 0
    source:
    {
        var url = settings.masterBackend + "Dvr/GetRecordedList";
        var first = true;

        if (count > 0)
        {
            if (first)
            {
                url = url + "?Count=" + count;
                first = false;
            }
            else
                url = url + "&Count=" + count;
        }

        if (title != "")
        {
            if (first)
            {
                url = url + "?TitleRegEx=" + title;
                first = false;
            }
            else
                url = url + "&TitleRegEx=" + title;
        }

        if (recGroup != "")
        {
            if (first)
            {
                url = url + "?RecGroup=" + recGroup;
                first = false;
            }
            else
                url = url + "&RecGroup=" + recGroup;
        }

        if (storageGroup != "")
        {
            if (first)
            {
                url = url + "?StorageGroup=" + storageGroup;
                first = false;
            }
            else
                url = url + "&StorageGroup=" + storageGroup;
        }

        console.log(url);

        return url;
    }

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
    XmlRole { name: "Season"; query: "Season/string()" }
    XmlRole { name: "Episode"; query: "Episode/string()" }
    XmlRole { name: "TotalEpisodes"; query: "TotalEpisodes/string()" }
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

    onStatusChanged:
    {
        if (status == XmlListModel.Ready)
        {
            console.log("Loaded Recordings: " + count);
        }

        if (status === XmlListModel.Loading)
        {
        }

        if (status === XmlListModel.Error)
        {

            console.log("Error: " + errorString + "\n \n \n " + recordingModel.source.toString());
        }
    }
}
