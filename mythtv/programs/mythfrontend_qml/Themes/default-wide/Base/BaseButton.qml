import QtQuick 2.0

Item
{
    property alias text: buttonText.text

    signal clicked();

    x: 0; y: 0; width: 200; height: 50
    focus: false

    Gradient
    {
        id: focusedGradient
        GradientStop { position: 0.0; color: "#c84d6b30" }
        GradientStop { position: 1.0; color: "#c858f833" }
    }

    states:
    [
        State
        {
            name: "active"
            PropertyChanges
            {
                target: background;
                color: "#78cccccc"
                gradient: {}
            }
        },
        State
        {
            name: "selectedactive"
            PropertyChanges
            {
                target: background;
                //color: "#ffffff"
                //opacity: 1
                gradient: focusedGradient
            }
        },
        State
        {
            name: "disabled"
            PropertyChanges
            {
                target: background;
                color: "#ffffff"
            }
        },
        State
        {
            name: "pushed"
            PropertyChanges
            {
                target: buttonText;
                font.pixelSize: 28
            }
            PropertyChanges
            {
                target: background;
                //color: "#ffffff"
                //opacity: 1
                gradient: focusedGradient
            }
        }
    ]

    onFocusChanged:
    {
        if (focus)
            state = "selectedactive"
        else
            state = "active"
    }

    Timer
    {
        id: pushTimer
        interval: 250; running: false;
        onTriggered:
        {
            if (focus) state = "selectedactive"; else state = "active";
        }
    }

    Keys.onReturnPressed:
    {
        state = "pushed"
        pushTimer.start();
        clicked();
    }

    Rectangle
    {
        id: background
        anchors.fill: parent
        color: "#78cccccc"
        border.width: 3
        border.color: "#ffffff"
        radius: 4
    }

    Text
    {
        id: buttonText
        anchors {fill: parent; margins: 3}
        font.family: "Droid Sans"
        font.pixelSize: 26
        font.bold: true
        color: "#ffffff"
        opacity: 1
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        clip: true
        elide: Text.ElideRight
    }
}

