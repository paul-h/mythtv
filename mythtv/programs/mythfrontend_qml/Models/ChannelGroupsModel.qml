import QtQuick 2.0
import QtQuick.XmlListModel 2.0

XmlListModel
{
    id: channelGroupsModel

    source: settings.masterBackend + "Guide/GetChannelGroupList"
    query: "/ChannelGroupList/ChannelGroups/ChannelGroup"

    XmlRole { name: "GroupId"; query: "GroupId/number()" }
    XmlRole { name: "Name"; query: "Name/string()" }
    XmlRole { name: "Password"; query: "Password/string()" }

    onStatusChanged:
    {
        if (status == XmlListModel.Ready)
        {
            console.log("Status: " + "ChannelGroups - Found " + count + " channelgroups");
        }

        if (status === XmlListModel.Loading)
        {
            console.log("Status: " + "ChannelGroups - LOADING - " + source.toString());
        }

        if (status === XmlListModel.Error)
        {
            console.log("Status: " + "ChannelGroups - ERROR: " + errorString() + "\n" + source.toString());
        }
    }
}
