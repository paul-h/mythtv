import QtQuick 2.4
import QtMultimedia 5.4
import QtQuick.Controls 1.4
import QtQuick.XmlListModel 2.0
import Process 1.0
import Base 1.0
import Screens 1.0
import "Models"

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

    // theme background video downloader
    Process
    {
        id: themeDLProcess
        onFinished:
        {
            if (exitStatus == Process.NormalExit)
            {
                screenBackground.showVideo = true;
                screenBackground.setVideo("file://" + theme.backgroundVideo);
                screenBackground.showImage = false;
            }
        }
    }

    // theme loader
    Loader
    {
        id: themeLoader
        source: settings.qmlPath + "Theme.qml"

        onLoaded:
        {
            if (theme.backgroundVideo != "")
            {
                if (theme.needsDownload && !mythUtils.fileExists(theme.backgroundVideo))
                {
                    screenBackground.showVideo = false;
                    screenBackground.showImage = true;
                    themeDLProcess.start(theme.downloadCommand, theme.downloadOptions);
                }
                else
                {
                    screenBackground.showVideo = true;
                    screenBackground.showImage = false;
                }
            }
            else
            {
                screenBackground.showVideo = false;
                screenBackground.showImage = true;
            }
        }
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

    Timer
    {
        id: tickerUpdateTimer
        interval: 60000; running: true; repeat: true
        onTriggered: tickerModel.reload();
    }

    TickerModel
    {
        id: tickerModel

        onStatusChanged:
        {
            if (status == XmlListModel.Ready)
            {
                screenBackground.clearTickerItems();

                for (var x = 0; x < count; x++)
                {
                    var text = "<b>" + get(x).category + ":</b> " + get(x).text;
                    screenBackground.addTickerItem(get(x).id, text);
                }
            }
        }
    }

    ScreenBackground
    {
        id: screenBackground
        showImage: true
        showVideo: false
        showTicker: true
        Component.onCompleted:
        {
            setTitle(true, "Main Menu");
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
            else if (event.key === Qt.Key_F10)
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
