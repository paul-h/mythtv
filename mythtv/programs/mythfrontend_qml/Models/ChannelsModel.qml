import QtQuick 2.0
import QtQuick.XmlListModel 2.0

XmlListModel
{
    id: channelsModel

    source: settings.masterBackend + "Channel/GetChannelInfoList?OnlyVisible=true&Details=true"
    query: "/ChannelInfoList/ChannelInfos/ChannelInfo"

    XmlRole { name: "ChanId"; query: "ChanId/string()" }
    XmlRole { name: "ChanNum"; query: "ChanNum/string()" }
    XmlRole { name: "CallSign"; query: "CallSign/string()" }
    XmlRole { name: "IconURL"; query: "IconURL/string()" }
    XmlRole { name: "ChannelName"; query: "ChannelName/string()" }
    XmlRole { name: "SourceId"; query: "SourceId/string()" }

    onStatusChanged:
    {
        if (status == XmlListModel.Ready)
        {
            console.info("Status: " + "Channels - Found " + count + " channels");
        }

        if (status === XmlListModel.Loading)
        {
            console.log("Status: " + "Channels - LOADING - " + source.toString());
        }

        if (status === XmlListModel.Error)
        {
            console.log("Status: " + "Channels - ERROR: " + errorString + "\n" + source.toString());
        }
    }
}
