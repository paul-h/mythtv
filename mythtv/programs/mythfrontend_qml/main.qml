import QtQuick 2.4
import QtMultimedia 5.4
import QtQuick.Controls 1.4
import "Themes/MythCenter-wide"
import "MenuThemes/classic"
import "Themes/MythCenter-wide/base"

ApplicationWindow
{
    id: window
    visible: true
    visibility: "Windowed"
    width: 1280
    height: 720

    // Global Settings
    Item
    {
        id: settings
        property string masterBackend: dbUtils.getSetting("Qml_masterBackend", hostName)
        property string videoPath:     dbUtils.getSetting("Qml_videoPath", hostName)
        property string picturePath:   dbUtils.getSetting("Qml_picturePath", hostName)
        property string sdChannels:    dbUtils.getSetting("Qml_sdChannels", hostName)

        // vbox
        property string vboxIP: dbUtils.getSetting("Qml_vboxIP", hostName)

        // hdmiEncoder
        property string hdmiEncoder: dbUtils.getSetting("Qml_hdmiEncoder", hostName)

        // infoFont properties
        property font infoFont: Qt.font(
        {
            family: "Liberation Sans",
            bold: false,
            italic: false,
            pixelSize: 22
        });
        property color infoFontColor: "white"

        // labelFont properties
        property font labelFont: Qt.font(
        {
            family: "Liberation Sans",
            bold: true,
            italic: false,
            pixelSize: 24
        });
        property color labelFontColor: "white"

        property double wmult: window.width / 1280
        property double hmult: window.height / 720
    }

    function xscale(x)
    {
        return x * settings.wmult;
    }

    function yscale(y)
    {
        return y * settings.hmult;
    }

    // Sound effects
    SoundEffect
    {
         id: upSound
         source: qmlPath + "sounds/pock.wav"
    }
    SoundEffect
    {
         id: downSound
         source: qmlPath + "sounds/pock.wav"
    }
    SoundEffect
    {
         id: leftSound
         source: qmlPath + "sounds/pock.wav"
    }
    SoundEffect
    {
         id: rightSound
         source: qmlPath + "sounds/pock.wav"
    }
    SoundEffect
    {
         id: returnSound
         source: qmlPath + "sounds/poguck.wav"
    }
    SoundEffect
    {
         id: escapeSound
         source: qmlPath + "sounds/pock.wav"
    }

    ScreenBackground
    {
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
}
