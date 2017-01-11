import QtQuick 2.0
import Base 1.0
import "../../../MenuThemes/classic"

BaseScreen
{
    property alias model: listView.model
    property alias logo: title.source
    defaultFocusItem: listView

    Component.onCompleted:
    {
        showTitle(true, model.title);
        showTime(true);
        showTicker(true);
        showVideo(true);
    }

    Image
    {
        id: title
        x: xscale(24); y: yscale(28)
        width: xscale(sourceSize.width)
        height: yscale(sourceSize.height)
        source: settings.themePath + "title/title_tv.png"
    }

    Image
    {
        id: logo
        x: xscale(30); y: yscale(594)
        width: xscale(sourceSize.width)
        height: yscale(sourceSize.height)
        source: settings.themePath + "ui/mythtv_logo.png"
    }

    Image
    {
        id: horizon
        x: xscale(550); y: yscale(500)
        width: xscale(sourceSize.width)
        height: yscale(sourceSize.height)
        source: settings.themePath + "ui/horizon.png"
    }

    Rectangle
    {
        x: xscale(200); y: yscale(150)
        width: xscale(500); height: yscale(360)
        color: "#00000000"
        Component
        {
            id: menuDelegate
            Item
            {
                width: parent.width; height: yscale(60)
                TitleText
                {
                    text: menutext
                    fontFamily: theme.menuFontFamily
                    fontPixelSize: xscale(theme.menuFontPixelSize)
                    fontBold: theme.menuFontBold
                    fontColor: theme.menuFontColor
                    shadowAlpha: theme.menuShadowAlpha
                    shadowColor: theme.menuShadowColor
                    shadowXOffset: theme.menuShadowXOffset
                    shadowYOffset: theme.menuShadowYOffset
                    x: xscale(10); width: parent.width - xscale(20);
                    anchors.verticalCenter: parent.verticalCenter
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: {
                        listView.currentIndex = index
                    }
                }
            }
        }

        ListView
        {
            id: listView
            width: parent.width; height: parent.height
            delegate: menuDelegate
            highlight: Image {source: settings.themePath + "ui/button_on.png"}
            focus: true
            clip: true
            keyNavigationWraps: true

            Keys.onPressed:
            {
                if (event.key === Qt.Key_PageDown)
                {
                    currentIndex = currentIndex + 6 >= model.count ? model.count - 1 : currentIndex + 6;
                    event.accepted = true;
                    downSound.play();
                }
                else if (event.key === Qt.Key_PageUp)
                {
                    currentIndex = currentIndex - 6 < 0 ? 0 : currentIndex - 6;
                    event.accepted = true;
                    upSound.play()
                }
                else if (event.key === Qt.Key_Up)
                {
                    event.accepted = false;
                    upSound.play()
                }
                else if (event.key === Qt.Key_Down)
                {
                    event.accepted = false;
                    downSound.play()
                }
            }

            Keys.onReturnPressed:
            {
                console.log("return pressed");
                event.accepted = true;
                returnSound.play();
                // FIXME this is hacky
                if (model.get(currentIndex).menutext === "Music")
                {
                    stack.push({item: Qt.resolvedUrl("ThemedMenu.qml"), properties:{model: musicMenu, logo: settings.themePath + "title/title_music.png" }});
                }
                else if (model.get(currentIndex).menutext === "Videos")
                {
                    stack.push({item: Qt.resolvedUrl("ThemedMenu.qml"), properties:{model: videoMenu, logo: settings.themePath + "title/title_video.png" }});
                }
                else if (model.get(currentIndex).menutext === "ZoneMinder")
                {
                    stack.push({item: Qt.resolvedUrl("ThemedMenu.qml"), properties:{model: zoneminderMenu, logo: settings.themePath + "title/title_keys.png" }});
                }
                else if (model.get(currentIndex).menutext === "Images")
                {
                    stack.push({item: Qt.resolvedUrl("IconView.qml"), properties:{folder: settings.picturePath }});
                }
                else if (model.get(currentIndex).menutext === "Setup")
                {
                    stack.push({item: Qt.resolvedUrl("ThemedMenu.qml"), properties:{model: settingsMenu, logo: settings.themePath + "title/title_keys.png" }});
                }
                else
                {
                    stack.push({item: Qt.resolvedUrl(model.get(currentIndex).loaderSource)})
                }

                event.accepted = true;
            }

            onCurrentItemChanged: watermark.swapImage(settings.themePath + model.get(currentIndex).waterMark)
        }
    }

    MusicMenu {id: musicMenu}
    VideoMenu {id: videoMenu}
    ZoneMinderMenu {id: zoneminderMenu }
    SettingsMenu {id: settingsMenu }
 
    FadeImage
    {
        id: watermark
        x: xscale(832); y: yscale(196); width: xscale(300); height: yscale(300)
        source: settings.themePath + "watermark/tv.png"
    }


    //    ReflectionImage {
    //        id: reflectionImage
    //        // main image
    //        x:832
    //        y:196
    //        width:  200; height: 200
    //    }

    //    Scroller {
    //        text: "<b>NEWS</b> This is some very loooooog ticker text that gets displayed whenever you're seeing this ticker here, bla bla bla..."
    //    }
}

