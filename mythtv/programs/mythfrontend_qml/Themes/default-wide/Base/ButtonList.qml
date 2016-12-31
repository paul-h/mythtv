import QtQuick 2.0

ListView
{
    id: root
    x: 100; y: 100; width: 1000; height: 500

    clip: true
    model: {}
    delegate: {}

    signal itemClicked(int index);
    signal itemSelected(int index);

    Keys.onPressed:
    {
        var rowCount = height / currentItem.height;

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
}
