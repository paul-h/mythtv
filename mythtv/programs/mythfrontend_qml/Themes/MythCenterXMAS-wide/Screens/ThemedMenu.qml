import QtQuick 2.0
import Base 1.0
import Screens 1.0
import "../../../MenuThemes/classic"

Item
{
    //anchors.fill: parent
    x: 0; y: 0; width: parent.width; height: parent.height
    property alias model: listView.model
    property alias logo: title.source
    property alias defaultFocusItem: listView

    Component.onCompleted:
    {
        screenBackground.setTitle(false, "");
        screenBackground.showTime = true;
    }

    Image
    {
        id: title
        x: xscale(24); y: yscale(28)
        width: xscale(sourceSize.width)
        height: yscale(sourceSize.height)
        source: settings.themePath + model.logo
    }

    Image
    {
        id: logo
        x: xscale(30); y: yscale(594)
        width: xscale(sourceSize.width)
        height: yscale(sourceSize.height)
        source: settings.themePath + "ui/mythtv_logo.png"
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


            Keys.onEscapePressed: if (stack.depth > 1) {stack.pop(); escapeSound.play();} else Qt.quit();
            Keys.onReturnPressed:
            {
                console.log("return pressed");
                event.accepted = true;
                returnSound.play();
                // FIXME this is hacky
                if (model.get(currentIndex).menutext === "Music")
                {
                    stack.push({item: createSubMenu(), properties:{model: musicMenu}});
                }
                else if (model.get(currentIndex).menutext === "Videos")
                {
                    stack.push({item: createSubMenu(), properties:{model: videoMenu}});
                }
                else if (model.get(currentIndex).menutext === "ZoneMinder")
                {
                    stack.push({item: createSubMenu(), properties:{model: zoneminderMenu}});
                }
                else if (model.get(currentIndex).menutext === "Images")
                {
                    stack.push({item: Qt.resolvedUrl("IconView.qml"), properties:{folder: settings.picturePath }});
                }
                else if (model.get(currentIndex).menutext === "Setup")
                {
                    stack.push({item: createSubMenu(), properties:{model: settingsMenu}});
                }
                else
                {
                    stack.push({item: Qt.resolvedUrl(model.get(currentIndex).loaderSource)})
                }

                event.accepted = true;
            }
        }
    }

    function createSubMenu()
    {
        var themeComponent;
        var menu;

        themeComponent = Qt.createComponent("ThemedMenu.qml");
        menu = themeComponent.createObject(window);

        if (menu == null) 
        {
            // Error Handling
            console.log("Error creating sub menu object" + themeComponent.errorString());
        }

        return menu;
    }

    MusicMenu {id: musicMenu}
    VideoMenu {id: videoMenu}
    ZoneMinderMenu {id: zoneminderMenu }
    SettingsMenu {id: settingsMenu }
  }

