import QtQuick 2.0
import Base 1.0

FocusScope
{
    id: modalDialog

    property string title: ""
    property string message: ""

    property alias filterTitle: titleedit.text
    property alias filterType: typeedit.text
    property alias filterGenres: genreedit.text

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
                height: yscale(50)
                text: message
                multiline: true
                horizontalAlignment: Text.AlignHCenter
            }
        }

        InfoText
        {
            x: 20; y: 100
            width: xscale(300); height: yscale(30)
            text: "Title"
        }

        BaseEdit
        {
            id: titleedit
            x: xscale(20); y: yscale(120); width: xscale(400);
            text: "";
            KeyNavigation.up: okButton;
            KeyNavigation.down: titleButton;
            onTextHasChanged: console.log("Title is now: " + text);
        }

        BaseButton
        {
            id: titleButton
            x: xscale(430); y: yscale(120); width: xscale(50); height: yscale(50)
            text: "*"

            KeyNavigation.left: titleedit;
            KeyNavigation.right: typeedit;

            onClicked:
            {
            }
        }

        InfoText
        {
            x: 20; y: 180
            width: xscale(300); height: yscale(30)
            text: "Type"
        }

        BaseEdit
        {
            id: typeedit
            x: xscale(20); y: yscale(200); width: xscale(400);
            text: "";
            KeyNavigation.up: titleedit;
            KeyNavigation.down: typeButton;
            onTextHasChanged: console.log("Type is now: " + text);
        }

        BaseButton
        {
            id: typeButton
            x: xscale(430); y: yscale(200); width: xscale(50); height: yscale(50)
            text: "*"

            KeyNavigation.left: titleedit;
            KeyNavigation.right: typeedit;

            onClicked:
            {
            }
        }

        InfoText
        {
            x: 20; y: 260
            width: xscale(250); height: yscale(30)
            text: "Genres"
        }

        BaseEdit
        {
            id: genreedit
            x: xscale(20); y: yscale(270); width: xscale(400);
            text: "";
            KeyNavigation.up: typeedit;
            KeyNavigation.down: genreButton;
            onTextHasChanged: console.log("Genre is now: " + text);
        }

        BaseButton
        {
            id: genreButton
            x: xscale(430); y: yscale(270); width: xscale(50); height: yscale(50)
            text: "*"

            KeyNavigation.left: genreedit;
            KeyNavigation.right: okButton;

            onClicked:
            {
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
