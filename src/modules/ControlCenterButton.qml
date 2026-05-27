import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

ToolButton
{
    id: controlCenterButton

    property var popup
    property bool useSystemThemeIcons: true
    property string networkIconName: "network-wireless"
    property string bluetoothIconName: "bluetooth-active"
    property string volumeIconName: "audio-volume-medium"
    property var glyphForIcon
    property var glyphColorForKind
    readonly property bool popupVisible: controlCenterButton.popup && controlCenterButton.popup.opened
    readonly property color activeContentColor: (controlCenterButton.down || controlCenterButton.checked) ? Maui.Theme.highlightedTextColor : Maui.Theme.textColor

    text: "Control center"
    display: ToolButton.IconOnly
    checked: popupVisible
    padding: Maui.Style.space.small
    property bool _skipNextClick: false
    property bool _manualCloseRequest: false
    onClicked:
    {
        if (_skipNextClick)
        {
            _skipNextClick = false
            return
        }

        if (controlCenterButton.popup && (controlCenterButton.popup.opened || controlCenterButton.popup.visible))
        {
            _manualCloseRequest = true
            controlCenterButton.popup.close()
        }
        else if (controlCenterButton.popup)
            controlCenterButton.popup.open()
    }

    Connections
    {
        target: controlCenterButton.popup

        function onAboutToHide()
        {
            if (!controlCenterButton._manualCloseRequest)
                controlCenterButton._skipNextClick = true
        }

        function onClosed()
        {
            controlCenterButton._manualCloseRequest = false
        }

        function onOpened()
        {
            controlCenterButton._skipNextClick = false
        }
    }

    contentItem: RowLayout
    {
        spacing: Maui.Style.space.medium

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
                source: controlCenterButton.networkIconName
                color: controlCenterButton.activeContentColor
                visible: controlCenterButton.useSystemThemeIcons
            }

            Label
            {
                anchors.centerIn: parent
                visible: !controlCenterButton.useSystemThemeIcons
                text: controlCenterButton.glyphForIcon ? controlCenterButton.glyphForIcon(controlCenterButton.networkIconName) : ""
                color: controlCenterButton.activeContentColor
                font.family: "Symbols Nerd Font"
                font.weight: Font.Normal
                font.pixelSize: Math.max(13, Math.round(parent.height * 0.9))
                textFormat: Text.PlainText
                renderType: Text.QtRendering
            }
        }

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
                source: controlCenterButton.bluetoothIconName
                color: controlCenterButton.activeContentColor
                visible: controlCenterButton.useSystemThemeIcons
            }

            Label
            {
                anchors.centerIn: parent
                visible: !controlCenterButton.useSystemThemeIcons
                text: controlCenterButton.glyphForIcon ? controlCenterButton.glyphForIcon(controlCenterButton.bluetoothIconName) : ""
                color: controlCenterButton.activeContentColor
                font.family: "Symbols Nerd Font"
                font.weight: Font.Normal
                font.pixelSize: Math.max(13, Math.round(parent.height * 0.9))
                textFormat: Text.PlainText
                renderType: Text.QtRendering
            }
        }

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
                source: controlCenterButton.volumeIconName
                color: controlCenterButton.activeContentColor
                visible: controlCenterButton.useSystemThemeIcons
            }

            Label
            {
                anchors.centerIn: parent
                visible: !controlCenterButton.useSystemThemeIcons
                text: controlCenterButton.glyphForIcon ? controlCenterButton.glyphForIcon(controlCenterButton.volumeIconName) : ""
                color: controlCenterButton.activeContentColor
                font.family: "Symbols Nerd Font"
                font.weight: Font.Normal
                font.pixelSize: Math.max(13, Math.round(parent.height * 0.9))
                textFormat: Text.PlainText
                renderType: Text.QtRendering
            }
        }
    }
}
