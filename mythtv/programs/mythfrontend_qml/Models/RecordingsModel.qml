import QtQuick 2.0
import QtQuick.XmlListModel 2.0

XmlListModel
{
    id: recordingModel
    source: settings.masterBackend + "Dvr/GetRecordedList"
    query: "/ProgramList/Programs/Program"
    XmlRole { name: "StartTime"; query: "StartTime/string()" }
    XmlRole { name: "EndTime"; query: "EndTime/string()" }
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
    XmlRole { name: "LastModified"; query: "LastModified/string()" }
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
