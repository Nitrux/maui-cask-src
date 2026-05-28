import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

ToolButton
{
    id: controlCenterButton

    property var popup
    property bool useSystemThemeIcons: true
    property string networkIconName
    property string bluetoothIconName
    property string volumeIconName
    property string volumePercentageText
    property string batteryIconName
    property string batteryPercentageText
    property string powerProfileIconName
    property var glyphForIcon
    property var glyphColorForKind
    property int reopenGuardMs: 180
    property double _lastClosedAtMs: -1
    readonly property bool popupVisible: controlCenterButton.popup && controlCenterButton.popup.visible
    readonly property color activeContentColor: (controlCenterButton.down || controlCenterButton.popupVisible) ? Maui.Theme.highlightedTextColor : Maui.Theme.textColor

    text: "Control center"
    display: ToolButton.IconOnly
    checked: popupVisible
    padding: Maui.Style.space.small
    function togglePopup()
    {
        if (!controlCenterButton.popup)
            return

        if (controlCenterButton.popup.visible)
        {
            controlCenterButton.popup.close()
            return
        }

        if (_lastClosedAtMs >= 0 && (Date.now() - _lastClosedAtMs) < reopenGuardMs)
            return

        controlCenterButton.popup.open()
    }

    onClicked:
    {
        togglePopup()
    }

    Connections
    {
        target: controlCenterButton.popup

        function onClosed()
        {
            controlCenterButton._lastClosedAtMs = Date.now()
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
                font.pixelSize: Math.max(10, Math.round(parent.height * 0.65))
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
                font.pixelSize: Math.max(10, Math.round(parent.height * 0.65))
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
                id: _powerProfileIcon
                width: 16
                height: 16
                source: controlCenterButton.powerProfileIconName
                color: controlCenterButton.activeContentColor
                visible: controlCenterButton.useSystemThemeIcons && valid
            }

            Label
            {
                anchors.centerIn: parent
                visible: !controlCenterButton.useSystemThemeIcons || !(_powerProfileIcon.valid && controlCenterButton.useSystemThemeIcons)
                text: controlCenterButton.glyphForIcon ? controlCenterButton.glyphForIcon(controlCenterButton.powerProfileIconName) : ""
                color: controlCenterButton.activeContentColor
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
            width: _volumeRow.implicitWidth
            height: 20

            RowLayout
            {
                id: _volumeRow
                anchors.centerIn: parent
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
                        font.pixelSize: Math.max(10, Math.round(parent.height * 0.65))
                        textFormat: Text.PlainText
                        renderType: Text.QtRendering
                    }
                }

                WorkspaceBadge
                {
                    Layout.alignment: Qt.AlignVCenter
                    visible: controlCenterButton.volumePercentageText.length > 0
                    badgeText: controlCenterButton.volumePercentageText
                    bridge: null
                }
            }
        }

        Item
        {
            Layout.alignment: Qt.AlignVCenter
            width: _batteryRow.implicitWidth
            height: 20

            RowLayout
            {
                id: _batteryRow
                anchors.centerIn: parent
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
                        source: controlCenterButton.batteryIconName
                        color: controlCenterButton.activeContentColor
                        visible: controlCenterButton.useSystemThemeIcons
                    }

                    Label
                    {
                        anchors.centerIn: parent
                        visible: !controlCenterButton.useSystemThemeIcons
                        text: controlCenterButton.glyphForIcon ? controlCenterButton.glyphForIcon(controlCenterButton.batteryIconName) : ""
                        color: controlCenterButton.activeContentColor
                        font.family: "Symbols Nerd Font"
                        font.weight: Font.Normal
                        font.pixelSize: Math.max(10, Math.round(parent.height * 0.65))
                        textFormat: Text.PlainText
                        renderType: Text.QtRendering
                    }
                }

                WorkspaceBadge
                {
                    Layout.alignment: Qt.AlignVCenter
                    visible: controlCenterButton.batteryPercentageText.length > 0
                    badgeText: controlCenterButton.batteryPercentageText
                    bridge: null
                }
            }
        }
    }
}
