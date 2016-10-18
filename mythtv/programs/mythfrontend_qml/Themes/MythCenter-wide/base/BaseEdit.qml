import QtQuick 2.0

FocusScope
{
    property alias text: editText.text

    signal textHasChanged();

    x: 0; y: 0; width: xscale(200); height: yscale(50)
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
        }
    ]

    onFocusChanged:
    {
        if (focus)
        {
            state = "selectedactive"
        }
        else
        {
            state = "active"
        }
    }

    Keys.onReturnPressed:
    {
        textHasChanged();
    }

    Rectangle
    {
        id: background
        anchors {fill: parent; margins: 0}
        color: "#78cccccc"
        border.width: 3
        border.color: "#ffffff"
        radius: 4
    }

    Rectangle
    {
        id: textBackground
        anchors {fill: parent; margins: 6}
        color: "#c8ffffff"
        radius: 4
    }

    TextInput
    {
        id: editText
        focus: true
        anchors {fill: parent; margins: 6}
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.family: "Helvetica"
        font.pointSize: 20
        color: "black"
    }
}

