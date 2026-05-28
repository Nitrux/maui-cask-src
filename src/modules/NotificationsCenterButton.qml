import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

ToolButton
{
    id: notificationsCenterButton

    property var popup
    property bool useSystemThemeIcons: true
    property string iconName: "notifications"
    property var glyphForIcon
    property string countText: "1"
    property int reopenGuardMs: 180
    property double _lastClosedAtMs: -1
    readonly property bool popupVisible: notificationsCenterButton.popup && notificationsCenterButton.popup.visible
    readonly property color activeContentColor: (notificationsCenterButton.down || notificationsCenterButton.popupVisible) ? Maui.Theme.highlightedTextColor : Maui.Theme.textColor

    text: "Notifications center"
    display: ToolButton.IconOnly
    checked: popupVisible
    padding: Maui.Style.space.small

    function togglePopup()
    {
        if (!notificationsCenterButton.popup)
            return

        if (notificationsCenterButton.popup.visible)
        {
            notificationsCenterButton.popup.close()
            return
        }

        if (_lastClosedAtMs >= 0 && (Date.now() - _lastClosedAtMs) < reopenGuardMs)
            return

        notificationsCenterButton.popup.open()
    }

    onClicked:
    {
        togglePopup()
    }

    Connections
    {
        target: notificationsCenterButton.popup

        function onClosed()
        {
            notificationsCenterButton._lastClosedAtMs = Date.now()
        }
    }

    contentItem: RowLayout
    {
        spacing: Maui.Style.space.tiny

        Item
        {
            Layout.alignment: Qt.AlignVCenter
            width: 20
            height: 20

            Maui.Icon
            {
                anchors.centerIn: parent
                width: 16
                height: 16
                source: notificationsCenterButton.iconName
                color: notificationsCenterButton.activeContentColor
                visible: notificationsCenterButton.useSystemThemeIcons
            }

            Label
            {
                anchors.centerIn: parent
                visible: !notificationsCenterButton.useSystemThemeIcons
                text: notificationsCenterButton.glyphForIcon ? notificationsCenterButton.glyphForIcon(notificationsCenterButton.iconName) : ""
                color: notificationsCenterButton.activeContentColor
                font.family: "Symbols Nerd Font"
                font.weight: Font.Normal
                font.pixelSize: Math.max(10, Math.round(parent.height * 0.65))
                textFormat: Text.PlainText
                renderType: Text.QtRendering
            }
        }

        Item
        {
            Layout.alignment: Qt.AlignVCenter
            width: _badgeRow.implicitWidth
            height: 20

            RowLayout
            {
                id: _badgeRow
                anchors.centerIn: parent
                spacing: 0

                WorkspaceBadge
                {
                    Layout.alignment: Qt.AlignVCenter
                    visible: notificationsCenterButton.countText.length > 0
                    badgeText: notificationsCenterButton.countText
                    bridge: null
                }
            }
        }
    }
}
