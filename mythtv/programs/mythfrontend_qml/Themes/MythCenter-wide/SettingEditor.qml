import QtQuick 2.0
import "base"
import "../../Models"

Item
{
    //property alias source: mediaplayer.source
    property alias defaultFocusItem: masterBEEdit

    x: 0; y: 0; width: parent.width; height: parent.height

    Keys.onEscapePressed: if (stack.depth > 1) {stack.pop();} else Qt.quit();

    Image
    {
        id: background
        source: themePath + "ui/background.png"
    }

    LabelText
    {
        x: xscale(50); y: yscale(100)
        text: "Master Backend IP:"
    }

    TextEdit
    {
        id: masterBEEdit
        x: xscale(500); y: yscale(100)
        width: xscale(500)
        height: yscale(25)
        text: settings.masterBackend
        font.family: "Helvetica"
        font.pointSize: xscale(20)
        color: "blue"
        KeyNavigation.up: saveButton
        KeyNavigation.down: videoPathEdit
    }

    LabelText
    {
        x: xscale(50); y: yscale(150)
        text: "Video Path:"
    }

    TextEdit
    {
        id: videoPathEdit
        x: xscale(500); y: yscale(150)
        width: xscale(500)
        height: yscale(25)
        text: settings.videoPath
        font.family: "Helvetica"
        font.pointSize: xscale(20)
        color: "blue"
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

    TextEdit
    {
        id: picturePathEdit
        x: xscale(500); y: yscale(200)
        width: xscale(500)
        height: yscale(25)
        text: settings.picturePath
        font.family: "Helvetica"
        font.pointSize: xscale(20)
        color: "blue"
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

    TextEdit
    {
        id: sdChannelsEdit
        x: xscale(500); y: yscale(250)
        width: xscale(500)
        height: yscale(25)
        text: settings.sdChannels
        font.family: "Helvetica"
        font.pointSize: xscale(20)
        color: "blue"
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

    TextEdit
    {
        id: vboxIPEdit
        x: xscale(500); y: yscale(300)
        width: xscale(500)
        height: yscale(25)
        text: settings.vboxIP
        font.family: "Helvetica"
        font.pointSize: xscale(20)
        color: "blue"
        KeyNavigation.up: sdChannelsEdit;
        KeyNavigation.down: hdmiEncoderEdit;
    }

    LabelText
    {
        x: xscale(50); y: yscale(350)
        text: "HDMI Encoder:"
    }

    TextEdit
    {
        id: hdmiEncoderEdit
        x: xscale(500); y: yscale(350)
        width: xscale(500)
        height: yscale(25)
        text: settings.hdmiEncoder
        font.family: "Helvetica"
        font.pointSize: xscale(20)
        color: "blue"
        KeyNavigation.up: vboxIPEdit;
        KeyNavigation.down: saveButton;
    }

    BaseButton
    {
        id: saveButton;
        x: xscale(900); y: yscale(630);
        text: "Save";
        KeyNavigation.up: vboxIPEdit
        KeyNavigation.down: masterBEEdit
        onClicked:
        {
            console.log("save button pressed");
            dbUtils.setSetting("Qml_masterBackend", hostName, masterBEEdit.text);
            dbUtils.setSetting("Qml_videoPath    ", hostName, videoPathEdit.text);
            dbUtils.setSetting("Qml_picturePath",   hostName, picturePathEdit.text);
            dbUtils.setSetting("Qml_sdChannels",    hostName, sdChannelsEdit.text);
            dbUtils.setSetting("Qml_vboxIP",        hostName, vboxIPEdit.text);
            dbUtils.setSetting("Qml_hdmiEncoder",   hostName, hdmiEncoderEdit.text);

            settings.masterBackend = masterBEEdit.text;
            settings.videoPath     = videoPathEdit.text;
            settings.picturePath   = picturePathEdit.text;
            settings.sdChannels    = sdChannelsEdit.text;
            settings.vboxIP        = vboxIPEdit.text;
            settings.hdmiEncoder   = hdmiEncoderEdit.text;

            returnSound.play();
            stack.pop();
        }
    }
}
