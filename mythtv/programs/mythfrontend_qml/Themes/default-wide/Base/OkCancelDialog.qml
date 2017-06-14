import QtQuick 2.0

FocusScope
{
    id: modalDialog

    property string title: ""
    property string message: ""

    property bool showCancelButton: true

    signal accepted
    signal cancelled

    width: 500
    height: 500
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    function show()
    {
        modalDialog.state = "show";
    }

    Keys.onEscapePressed:
    {
        modalDialog.state = "";
        modalDialog.cancelled();
    }

    Item
    {
        id: dialog

        opacity: 0
        anchors.fill: parent

        BaseDialogBackground
        {
            id: background
            anchors.fill: parent
        }

        Column
        {
            id: column
            width: parent.width
            height: 500
            spacing: 10

            TitleText
            {
                id: titleText
                x: 20; y: 5
                width: parent.width - 40
                text: title
                horizontalAlignment: Text.AlignHCenter
            }

            InfoText
            {
                id: messageText
                x: 20; y: 5
                width: parent.width - 40
                height: 150
                text: message
                multiline: true
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Row
        {
            x:0; y: parent.height - 60
            spacing: 10
            anchors
            {
                horizontalCenter: parent.horizontalCenter
                bottomMargin: 25
            }

            BaseButton
            {
                id: okButton
                text: "OK"
                focus: true

                KeyNavigation.left: cancelButton;
                KeyNavigation.right: cancelButton;

                onClicked:
                {
                    modalDialog.state = "";
                    modalDialog.accepted();
                }
            }

            BaseButton
            {
                id: cancelButton
                text: "Cancel"
                visible: modalDialog.showCancelButton

                KeyNavigation.left: okButton;
                KeyNavigation.right: okButton;

                onClicked:
                {
                    modalDialog.state = "";
                    modalDialog.cancelled();
                }
            }
        }
    }

    states:
    [
        State
        {
            name: ""
            PropertyChanges
            {
                target: dialog
                opacity: 0
            }
            PropertyChanges
            {
                target: modalDialog
                focus: false
            }
        },
        State
        {
            name: "show"
            PropertyChanges
            {
                target: dialog
                opacity: 1
            }
            PropertyChanges
            {
                target: modalDialog
                focus: true
            }
        }
    ]

    transitions:
    [
        Transition
        {
            from: ""
            to: "show"
            SequentialAnimation
            {
                NumberAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: 750 }
            }
        },
        Transition
        {
            from: "show"
            to: ""
            NumberAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: 750 }
        }
    ]
}
