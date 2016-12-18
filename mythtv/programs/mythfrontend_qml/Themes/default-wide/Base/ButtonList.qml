import QtQuick 2.0

ListView
{
    id: root
    x: 100; y: 100; width: 1000; height: 500

    clip: true
    model: {}
    delegate: {}
    highlight: highlight1

    signal itemClicked(int index);
    signal itemSelected(int index);

    states:
    [
        State
        {
            name: "active"
//            PropertyChanges
//            {
//                target: highlight.highlightRect;
//                color: "#78cccccc"
//                gradient: {}
//            }
        },
        State
        {
            name: "selectedactive"
//            PropertyChanges
//            {
//                target: highlight1.highlightRect;
//                //color: "#ffffff"
//                //opacity: 1
//                gradient: focusedGradient
//            }
        },
        State
        {
            name: "disabled"
//            PropertyChanges
//            {
//                target: highlight1.highlightRect;
//                color: "#ffffff"
//            }
        },
        State
        {
            name: "pushed"
//            PropertyChanges
//            {
//                target: buttonText;
//                font.pixelSize: 28
//            }
//            PropertyChanges
//            {
//                target: highlight1.highlightRect;
//                //color: "#ffffff"
//                //opacity: 1
//                gradient: focusedGradient
//            }
        }
    ]

    onFocusChanged:
    {
        if (focus)
            state = "selectedactive"
        else
            state = "active"
    }

    Keys.onPressed:
    {
        var rowCount = height / delegate.height;

        if (event.key === Qt.Key_PageDown)
        {
            currentIndex = currentIndex + rowCount >= model.count ? model.count - 1 : currentIndex + rowCount;
            event.accepted = true;
        }
        else if (event.key === Qt.Key_PageUp)
        {
            currentIndex = currentIndex - rowCount < 0 ? 0 : currentIndex - rowCount;
            event.accepted = true;
        }
    }

    Keys.onReturnPressed:
    {
        returnSound.play();
        itemClicked(currentIndex);
    }

    onCurrentItemChanged: itemSelected(currentIndex);

    Gradient
    {
        id: focusedGradient
        GradientStop { position: 0.0; color: "#c84d6b30" }
        GradientStop { position: 1.0; color: "#c858f833" }
    }

    Component
    {
        id: highlight1

        Rectangle
        {
            id: highlightRect
            width: parent.width; height: 50
            color: "black"
            opacity: 0.4
            radius: 25
            border.color: if (root.state === "selectedactive") "green"; else "white";
            border.width: 3
        }
    }
}
