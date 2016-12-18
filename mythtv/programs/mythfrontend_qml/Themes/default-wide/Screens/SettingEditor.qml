import QtQuick 2.0
import Base 1.0
import "../../../Models"

Item
{
    property alias defaultFocusItem: masterBEEdit

    x: 0; y: 0; width: parent.width; height: parent.height

    Keys.onEscapePressed: if (stack.depth > 1) {stack.pop();} else Qt.quit();

    Component.onCompleted:
    {
        screenBackground.showTitle = false
    }

    Component.onDestruction:
    {
        screenBackground.showTitle = true;
    }

    LabelText
    {
        x: xscale(50); y: yscale(100)
        text: "Master Backend IP:"
    }

    BaseEdit
    {
        id: masterBEEdit
        x: xscale(400); y: yscale(100)
        width: xscale(700)
        height: yscale(50)
        text: settings.masterBackend
        KeyNavigation.up: saveButton
        KeyNavigation.down: videoPathEdit
    }

    LabelText
    {
        x: xscale(50); y: yscale(150)
        text: "Video Path:"
    }

    BaseEdit
    {
        id: videoPathEdit
        x: xscale(400); y: yscale(150)
        width: xscale(700)
        height: yscale(50)
        text: settings.videoPath
        KeyNavigation.up: masterBEEdit;
        KeyNavigation.right: videoPathButton
        KeyNavigation.down: picturePathEdit;
    }

    BaseButton
    {
        id: videoPathButton;
        x: xscale(1120); y: yscale(150);
        width: xscale(50); height: yscale(50)
        text: "X";
        KeyNavigation.up: masterBEEdit
        KeyNavigation.left: videoPathEdit
        KeyNavigation.down: picturePathEdit;
        onClicked:
        {
            // TODO show directory finder popup
        }
    }

    //
    LabelText
    {
        x: xscale(50); y: yscale(200)
        text: "Picture Path:"
    }

    BaseEdit
    {
        id: picturePathEdit
        x: xscale(400); y: yscale(200)
        width: xscale(700)
        height: yscale(50)
        text: settings.picturePath
        KeyNavigation.up: videoPathEdit;
        KeyNavigation.right: picturePathButton
        KeyNavigation.down: sdChannelsEdit;
    }

    BaseButton
    {
        id: picturePathButton;
        x: xscale(1120); y: yscale(200);
        width: xscale(50); height: yscale(50)
        text: "X";
        KeyNavigation.up: videoPathEdit
        KeyNavigation.left: picturePathEdit
        KeyNavigation.down: sdChannelsEdit;
        onClicked:
        {
            // TODO show directory finder popup
        }
    }

    LabelText
    {
        x: xscale(50); y: yscale(250)
        text: "SD Channels File:"
    }

    BaseEdit
    {
        id: sdChannelsEdit
        x: xscale(400); y: yscale(250)
        width: xscale(700)
        height: yscale(50)
        text: settings.sdChannels
        KeyNavigation.up: picturePathEdit;
        KeyNavigation.right: sdChannelsButton
        KeyNavigation.down: vboxIPEdit;
    }

    BaseButton
    {
        id: sdChannelsButton;
        x: xscale(1120); y: yscale(250);
        width: xscale(50); height: yscale(50)
        text: "X";
        KeyNavigation.up: picturePathEdit
        KeyNavigation.left: sdChannelsEdit
        KeyNavigation.down: vboxIPEdit;
        onClicked:
        {
            // TODO show directory finder popup
        }
    }

    LabelText
    {
        x: xscale(50); y: yscale(300)
        text: "VBox IP:"
    }

    BaseEdit
    {
        id: vboxIPEdit
        x: xscale(400); y: yscale(300)
        width: xscale(700)
        height: yscale(50)
        text: settings.vboxIP
        KeyNavigation.up: sdChannelsEdit;
        KeyNavigation.down: hdmiEncoderEdit;
    }

    LabelText
    {
        x: xscale(50); y: yscale(350)
        text: "HDMI Encoder:"
    }

    BaseEdit
    {
        id: hdmiEncoderEdit
        x: xscale(400); y: yscale(350)
        width: xscale(700)
        height: yscale(50)
        text: settings.hdmiEncoder
        KeyNavigation.up: vboxIPEdit;
        KeyNavigation.down: themeEdit;
    }

    LabelText
    {
        x: xscale(50); y: yscale(400)
        text: "Theme:"
    }

    BaseEdit
    {
        id: themeEdit
        x: xscale(400); y: yscale(400)
        width: xscale(700)
        height: yscale(50)
        text: settings.themeName
        KeyNavigation.up: hdmiEncoderEdit;
        KeyNavigation.down: saveButton;
    }

    BaseButton
    {
        id: saveButton;
        x: xscale(900); y: yscale(630);
        text: "Save";
        KeyNavigation.up: themeEdit
        KeyNavigation.down: masterBEEdit
        onClicked:
        {
            console.log("save button pressed");
            dbUtils.setSetting("Qml_masterBackend", settings.hostName, masterBEEdit.text);
            dbUtils.setSetting("Qml_videoPath    ", settings.hostName, videoPathEdit.text);
            dbUtils.setSetting("Qml_picturePath",   settings.hostName, picturePathEdit.text);
            dbUtils.setSetting("Qml_sdChannels",    settings.hostName, sdChannelsEdit.text);
            dbUtils.setSetting("Qml_vboxIP",        settings.hostName, vboxIPEdit.text);
            dbUtils.setSetting("Qml_hdmiEncoder",   settings.hostName, hdmiEncoderEdit.text);
            dbUtils.setSetting("Qml_theme",         settings. hostName, themeEdit.text);

            settings.masterBackend = masterBEEdit.text;
            settings.videoPath     = videoPathEdit.text;
            settings.picturePath   = picturePathEdit.text;
            settings.sdChannels    = sdChannelsEdit.text;
            settings.vboxIP        = vboxIPEdit.text;
            settings.hdmiEncoder   = hdmiEncoderEdit.text;
            settings.themeName     = themeEdit.text;

            returnSound.play();
            stack.pop();
        }
    }
}
