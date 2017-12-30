import QtQuick 2.0
import QtQuick.XmlListModel 2.0

XmlListModel
{
    id: channelsModel

    property int sourceId

    source: settings.masterBackend + "Channel/GetChannelInfoList?SourceID=" + sourceId + "&OnlyVisible=true&Details=false"
    query: "/ChannelInfoList/ChannelInfos/ChannelInfo"

    XmlRole { name: "ChanId"; query: "ChanId/string()" }
    XmlRole { name: "ChanNum"; query: "ChanNum/string()" }
    XmlRole { name: "CallSign"; query: "CallSign/string()" }
    XmlRole { name: "IconURL"; query: "IconURL/string()" }
    XmlRole { name: "ChannelName"; query: "ChannelName/string()" }
    onStatusChanged:
    {
        if (status == XmlListModel.Ready)
        {
            console.log("Status: " + "Channels - READY \n" + channelsModel.source.toString());
        }

        if (status === XmlListModel.Loading)
        {
            console.log("Status: " + "Channels - LOADING \n" + channelsModel.source.toString());
        }

        if (status === XmlListModel.Error)
        {
            console.log("Status: " + "Channels - ERROR: " + errorString + "\n" + channelsModel.source.toString());
        }
    }
}
