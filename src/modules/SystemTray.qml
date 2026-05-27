import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

RowLayout
{
    id: systemTray

    spacing: Maui.Style.space.tiny

    ToolButton
    {
        icon.name: "org.kde.plasma.notifications"
        display: ToolButton.IconOnly
        flat: true
        focusPolicy: Qt.NoFocus
        onClicked: {}
        ToolTip.visible: hovered
        ToolTip.text: "SNI: Notifications"
    }

    ToolButton
    {
        icon.name: "network-vpn"
        display: ToolButton.IconOnly
        flat: true
        focusPolicy: Qt.NoFocus
        onClicked: {}
        ToolTip.visible: hovered
        ToolTip.text: "SNI: VPN"
    }
}
