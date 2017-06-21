import QtQuick 2.0
import QtQuick.XmlListModel 2.0

XmlListModel
{
    id: videoModel
    source: settings.masterBackend + "Video/GetVideoList"
    query: "/VideoMetadataInfoList/VideoMetadataInfos/VideoMetadataInfo"
    XmlRole { name: "Id"; query: "Id/string()" }
    XmlRole { name: "Title"; query: "Title/string()" }
    XmlRole { name: "SubTitle"; query: "SubTitle/string()" }
    XmlRole { name: "Tagline"; query: "Tagline/string()" }
    XmlRole { name: "Director"; query: "Director/string()" }
    XmlRole { name: "Studio"; query: "Studio/string()" }
    XmlRole { name: "Description"; query: "Description/string()" }
    XmlRole { name: "Inetref"; query: "Inetref/string()" }
    XmlRole { name: "Collectionref"; query: "Collectionref/string()" }
    XmlRole { name: "HomePage"; query: "HomePage/string()" }
    XmlRole { name: "ReleaseDate"; query: "ReleaseDate/string()" }
    XmlRole { name: "AddDate"; query: "AddDate/string()" }
    XmlRole { name: "UserRating"; query: "UserRating/string()" }
    XmlRole { name: "Length"; query: "Length/string()" }
    XmlRole { name: "PlayCount"; query: "PlayCount/string()" }
    XmlRole { name: "Season"; query: "Season/string()" }
    XmlRole { name: "Episode"; query: "Episode/string()" }
    XmlRole { name: "ParentalLevel"; query: "ParentalLevel/string()" }
    XmlRole { name: "Visible"; query: "Visible/string()" }
    XmlRole { name: "Watched"; query: "Watched/string()" }
    XmlRole { name: "Processed"; query: "Processed/string()" }
    XmlRole { name: "ContentType"; query: "ContentType/string()" }
    XmlRole { name: "Genre"; query: "string-join(Genres/GenreList/Genre/Name, ', ')" }
    XmlRole { name: "FileName"; query: "FileName/string()" }
    XmlRole { name: "Hash"; query: "Hash/string()" }
    XmlRole { name: "HostName"; query: "HostName/string()" }
    XmlRole { name: "Coverart"; query: "Coverart/string()" }
    XmlRole { name: "Fanart"; query: "Fanart/string()" }
    XmlRole { name: "Banner"; query: "Banner/string()" }
    XmlRole { name: "Screenshot"; query: "Screenshot/string()" }
    XmlRole { name: "Trailer"; query: "Trailer/string()" }

    signal loaded();

    onStatusChanged:
    {
        if (status == XmlListModel.Ready)
        {
            //screenBackground.showBusyIndicator = false;

            loaded();
        }

        if (status === XmlListModel.Loading)
        {
            //screenBackground.showBusyIndicator = true;
        }

        if (status === XmlListModel.Error)
        {
            //screenBackground.showBusyIndicator = false;

            console.log("Error: " + errorString + "\n \n \n " + videoModel.source.toString());
        }
    }
}
