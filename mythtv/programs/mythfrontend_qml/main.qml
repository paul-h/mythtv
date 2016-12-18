import QtQuick 2.4
import QtMultimedia 5.4
import QtQuick.Controls 1.4
import "MenuThemes/classic"
import Base 1.0
import Screens 1.0

ApplicationWindow
{
    id: window
    visible: true
    visibility: "Windowed"
    width: 1280
    height: 720

    property double wmult: width / 1280
    property double hmult: height / 720
    property alias theme: themeLoader.item

    // theme loader
    Loader
    {
        id: themeLoader
        source: settings.qmlPath + "Theme.qml"
    }

//    Component.onCompleted:
//    {
//        themeLoader.source = settings.qmlPath + "base/" + settings.themeName
//    }


    function xscale(x)
    {
        return x * wmult;
    }

    function yscale(y)
    {
        return y * hmult;
    }

    // Sound effects
    SoundEffect
    {
         id: upSound
         source: mythUtils.findThemeFile("sounds/pock.wav");
    }
    SoundEffect
    {
         id: downSound
         source: mythUtils.findThemeFile("sounds/pock.wav");
    }
    SoundEffect
    {
         id: leftSound
         source: mythUtils.findThemeFile("sounds/pock.wav")
    }
    SoundEffect
    {
         id: rightSound
         source: mythUtils.findThemeFile("sounds/pock.wav")
    }
    SoundEffect
    {
         id: returnSound
         source: mythUtils.findThemeFile("sounds/poguck.wav")
    }
    SoundEffect
    {
         id: escapeSound
         source: mythUtils.findThemeFile("sounds/pock.wav")
    }

    ScreenBackground
    {
        id: screenBackground
        title: "Main Menu"
        showTitle: true
        showImage: false
        showVideo: true
    }

    StackView
    {
        id: stack
        width: parent.width; height: parent.height
        initialItem: ThemedMenu {model: MainMenu {}}
        focus: true

        onCurrentItemChanged:
        {
            if (currentItem)
            {
                currentItem.defaultFocusItem.focus = true
            }
        }

        Keys.onPressed:
        {
            if (event.key === Qt.Key_F)
            {
                console.log("F pressed: " + visibility);
                if (visibility == 5)
                    visibility = 2
                else
                    visibility = 5
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent
        enabled: false
        cursorShape: Qt.BlankCursor
    }
}
