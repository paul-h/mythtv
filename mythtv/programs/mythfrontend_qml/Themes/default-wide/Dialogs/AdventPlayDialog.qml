import QtQuick 2.0
import Base 1.0

BaseDialog
{
    id: playDialog

    content: Item
    {
        anchors.fill: parent
    }

    buttons:
    [
        BaseButton
        {
            id: acceptButton
            text: "Play"
            focus: true
            visible: text != ""

            KeyNavigation.left: rejectButton;
            KeyNavigation.right: rejectButton;

            onClicked:
            {
                playDialog.state = "";
                playDialog.accepted();
            }
        },

        BaseButton
        {
            id: rejectButton
            text: "Cancel"
            visible: text != ""

            KeyNavigation.left: acceptButton;
            KeyNavigation.right: acceptButton;

            onClicked:
            {
                playDialog.state = "";
                playDialog.cancelled();
            }
        }
    ]
}

