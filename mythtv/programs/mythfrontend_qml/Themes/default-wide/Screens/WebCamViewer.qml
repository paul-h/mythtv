import QtQuick 2.0
import Base 1.0
import Dialogs 1.0

BaseScreen
{
    defaultFocusItem: webcamGrid

    Component.onCompleted:
    {
        showTitle(true, "WebCam Viewer");
    }

    ListModel
    {
        id: webcamModel
        ListElement
        {
            webcam: "1"
            title: "Railcam Free View Channel"
            icon: "https://i.ytimg.com/vi/bSaz5KDqVgg/hqdefault.jpg"
            //video: "https://www.youtube.com/watch?v=bSaz5KDqVgg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=bSaz5KDqVgg&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "2"
            title: "Dawlish Beach Cams, Blenheim Cam"
            icon: "https://i.ytimg.com/vi/tf7GnMEH2T8/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=tf7GnMEH2T8&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "3"
            title: "Dawlish Beach Cams, Salty Cottage Cam, Teignmouth"
            icon: "https://i.ytimg.com/vi/nE2DizZtb4U/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=nE2DizZtb4U&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "4"
            title: "LIVE Trains Railway 24/24 Train Driver's View: Cab Ride Line Railroad in Winter!"
            icon: "https://i.ytimg.com/vi/eFtWa01E1-o/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=eFtWa01E1-o&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "5"
            title: "Molti Treni Senza Sosta"
            icon: "https://i.ytimg.com/vi/9rZEAnuj1mI/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=9rZEAnuj1mI&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "6"
            title: "Riding The Rails TV - 24/7 Cab Ride Views from around the world!"
            icon: "https://i.ytimg.com/vi/E_-rumvbqdo/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=E_-rumvbqdo&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "7"
            title: "Bridport Harbour (West Bay)"
            icon: "https://i.ytimg.com/vi/vnlOhiN-bBI/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=vnlOhiN-bBI&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "8"
            title: "La Grange, Kentucky USA - Virtual Railfan LIVE"
            icon: "https://i.ytimg.com/vi/8OE1aS91yvQ/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=8OE1aS91yvQ&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "9"
            title: "La Plata, Missouri USA - Virtual Railfan LIVE"
            icon: "https://i.ytimg.com/vi/l06NXHeXIs8/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=l06NXHeXIs8&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "10"
            title: "Hastings Pier Webcam and English Channel LIVE HD"
            icon: "https://i.ytimg.com/vi/aRWsI8l-n5E/hqdefault.jpg"
            video: "https://www.youtube.com/TV#/watch/video/control?v=aRWsI8l-n5E&resume"
            player: "YouTube"
        }
        ListElement
        {
            webcam: "11"
            title: "Jersey Beach Cam - St. Ouen Bay"
            icon: "https://www.freedomholidays.com/_common/updateable/images/property/dsc_87822219a203d7.jpg?v=9160518"
            video: "https://portal.hdontap.com/s/embed/?stream=watersplash_visitjersey-CUST&ratio=16:9"
            player: "WebBrowser"
        }
        ListElement
        {
            webcam: "12"
            title: "Blackpool - Promenade"
            icon: "https://upload.wikimedia.org/wikipedia/commons/1/12/Blackpool_promenade_-_DSC07199.JPG"
            video: "https://www.skylinewebcams.com/webcam/united-kingdom/england/blackpool/blackpool.html?w=227"
            player: "WebBrowser"
        }
    }

    GridView
    {
        id: webcamGrid
        x: xscale(50)
        y: yscale(50)
        width: xscale(1280) - xscale(96)
        height: yscale(720) - yscale(100)
        cellWidth: xscale(197)
        cellHeight: yscale(155)

        Component
        {
            id: webcamDelegate
            Image
            {
                id: wrapper
                //visible: opened
                x: xscale(5)
                y: yscale(5)
                opacity: 1.0
                width: webcamGrid.cellWidth - 10; height: webcamGrid.cellHeight - 10
                source: icon
            }
        }

        highlight: Rectangle { z: 99; color: "red"; opacity: 0.4; radius: 5 }
        model: webcamModel
        delegate: webcamDelegate
        focus: true

        Keys.onReturnPressed:
        {
            returnSound.play();
            if (webcamGrid.model.get(webcamGrid.currentIndex).player === "YouTube")
                stack.push({item: Qt.resolvedUrl("YouTube.qml"), properties:{url: webcamGrid.model.get(webcamGrid.currentIndex).video}});
            else if (webcamGrid.model.get(webcamGrid.currentIndex).player === "WebBrowser")
            {
                var url = webcamGrid.model.get(webcamGrid.currentIndex).video
                stack.push({item: Qt.resolvedUrl("WebBrowser.qml"), properties:{url: url, fullscreen: true, zoom: 1.0}});
            }
                else
                stack.push({item: Qt.resolvedUrl("InternalPlayer.qml"), properties:{source1: webcamGrid.model.get(webcamGrid.currentIndex).video, title1: webcamGrid.model.get(webcamGrid.currentIndex).title}});
            event.accepted = true;
        }

        Keys.onPressed:
        {
            if (event.key === Qt.Key_M)
            {
                popupMenu.clearMenuItems();

                popupMenu.addMenuItem("Close All Windows");

                popupMenu.show();
            }
            else
            {
                event.accepted = false;
            }
        }
    }

    PopupMenu
    {
        id: popupMenu

        title: "Menu"
        message: "Webcam Viewer Options"

        onItemSelected:
        {
            webcamGrid.focus = true;

            if (itemText == "Close All Windows")
            {
                //TODO
            }
        }

        onCancelled:
        {
            webcamGrid.focus = true;
        }
    }
}
