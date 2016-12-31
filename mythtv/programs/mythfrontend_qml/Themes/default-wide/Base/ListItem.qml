import QtQuick 2.0

Item
{
    id: container

    property string fontName: "Helvetica"
    property int    fontSize: 10
    property color  fontColor: "black"
    property bool   fontBold: false
    property string text: "NOT SET"
    property string bgImage: mythUtils.findThemeFile("images/list_item.png")

    property string bgImageSelected: mythUtils.findThemeFile("images/list_item_selected.png")
    property string bgImagePressed: mythUtils.findThemeFile("images/list_item_pressed.png")
    property string bgImageActive: mythUtils.findThemeFile("images/list_item_active.png")
    property bool   selected: false
    property bool   selectable: true
    property int    textIndent: 0
    signal clicked

    width: 360
    height: 64
    clip: true
    onSelectedChanged: selected ? state = 'selected' : state = ''

    Rectangle
    {
        id: background
        anchors.fill: parent
        color:
        {
            if (parent.focused)
            {
                if (parent.selected)
                    theme.lvRowBackgroundFocusedSelected;
                else
                    theme.lvRowBackgroundFocused;
            }
            else
            {
                if (parent.selected)
                    theme.lvRowBackgroundSelected;
                else
                    theme.lvRowBackgroundNormal;
            }
        }

        border.color: theme.lvBackgroundBorderColor
        border.width: theme.lvBackgroundBorderWidth
        radius: theme.lvBackgroundBorderRadius
    }

    Text
    {
        id: itemText
        anchors
        {
            left: parent.left
            top: parent.top
            right: parent.right
            topMargin: 4
            bottomMargin: 4
            leftMargin: 8 + textIndent
            rightMargin: 8
            verticalCenter: container.verticalCenter
        }
        font
        {
            family: container.fontName
            pointSize: container.fontSize
            bold: container.fontBold
        }
        color: container.fontColor
        elide: Text.ElideRight
        text: container.text
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea
    {
        id: mouseArea
        anchors.fill: parent
        onClicked: container.clicked();
        onReleased: selectable && !selected ? selected = true : selected = false
    }

    states:
    [
        State 
        {
            name: 'pressed'; when: mouseArea.pressed
            //PropertyChanges { target: background; gradient: bgImagePressed;}
        },
        State
        {
            name: 'selected'
            //PropertyChanges { target: background; source: bgImageSelected; border { left: 35; top: 35; right: 35; bottom: 10 } }
        },
        State
        {
            name: 'active';
            //PropertyChanges { target: background; source: bgImageActive; }
        }
    ]
}
