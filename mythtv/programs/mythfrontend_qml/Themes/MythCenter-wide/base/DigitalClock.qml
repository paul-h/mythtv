import QtQuick 2.0

Item
{
    id: clock
    property string format: "ddd MMMM d yyyy , hh:mm ap"
    width: 200
    height: 40

    Text
    {
        id: time
        color: "darkgrey"
        text: Qt.formatDateTime(new Date(), parent.format)
        font.pixelSize:  30
        anchors.fill: parent

        function timeChanged()
        {
            time.text = Qt.formatDateTime(new Date(), parent.format)
        }

        Timer
        {
            interval: 500; running: true; repeat: true;
            onTriggered: time.timeChanged()
        }
    }
}

