import QtQuick 2.0

Rectangle
{
    anchors.fill: parent
    color:
    {
        /*
        if (parent.focused)
        {
            if (parent.selected)
                Theme.lvRowBackgroundSelected;
            else
                Theme.lvRowBackgroundFocused;
        }
        else
        {
            if (parent.selected)
                Theme.lvRowBackgroundActive;
            else
                Theme.lvRowBackgroundNormal;
        }
*/
        
        //if (parent.selected)
        //    Theme.lvRowBackgroundActive;
        //else
            theme.lvRowBackgroundNormal;
    }

    opacity: theme.lvBackgroundOpacity
    border.color: theme.lvBackgroundBorderColor
    border.width: theme.lvBackgroundBorderWidth
    radius: theme.lvBackgroundBorderRadius
}

