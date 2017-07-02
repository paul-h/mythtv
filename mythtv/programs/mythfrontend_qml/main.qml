import QtQuick 2.4
import QtMultimedia 5.4
import QtQuick.Controls 1.4
import Base 1.0
import Screens 1.0

ApplicationWindow
{
    id: window
    visible: true
    visibility: settings.startFullscreen ? "FullScreen" : "Windowed"
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

    function showMouse(show)
    {
         if (show)
             mouseArea.cursorShape = Qt.Arrow
         else
             mouseArea.cursorShape = Qt.BlankCursor
    }

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
        showImage: false
        showVideo: true
        showTicker: true
        Component.onCompleted:
        {
            setTitle(true, "Main Menu");
            addTickerItem(1, "<b>NEWS:</b> This is some very loooooog ticker text that gets displayed whenever you're seeing this ticker here, bla bla bla...");
            addTickerItem(2, "<b>RECORDING:</b> 1973. Sid and Babs together again for another classic British festival of innuendo. A local businessman convinces the town council of a seaside resort to stage a beauty contest.");
            addTickerItem(3, "<b>WEATHER:</b> Most places dry today. Clear and frosty tonight. A return to colder conditions with sunshine for the north and east of the UK; just a few coastal wintry showers and strong winds along North Sea coasts. Cloudier elsewhere with patchy rain or drizzle but skies clearing later on.");
            addTickerItem(4, "<b>SPORT: </b> Sunderland have rejected West Ham's £6m bid for striker Jermain Defoe");
        }
    }

    Loader
    {
        id: mainMenuLoader
        source: settings.menuPath + "MainMenu.qml"
    }

    StackView
    {
        id: stack
        width: parent.width; height: parent.height
        initialItem: ThemedMenu {model: mainMenuLoader.item}
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
                if (visibility == 5)
                    visibility = 2
                else
                    visibility = 5
            }
            else if (event.key === Qt.Key_F12)
            {
                settings.showTextBorder = ! settings.showTextBorder;
                console.log("showTextBorder: " + settings.showTextBorder);
            }
            else if (event.key === Qt.Key_F11)
            {
                if (stack.depth > 1) {stack.pop(); escapeSound.play();} else Qt.quit();
            }
        }
    }

    MouseArea
    {
        id: mouseArea
        anchors.fill: parent
        enabled: false
        cursorShape: Qt.BlankCursor
    }
}
