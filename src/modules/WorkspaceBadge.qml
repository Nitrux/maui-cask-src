import QtQuick
import QtQuick.Controls

import org.mauikit.controls as Maui

ToolButton
{
    id: workspaceBadge

    property QtObject bridge

    text: workspaceBadge.bridge ? String(workspaceBadge.bridge.currentWorkspace) : "1"
    display: ToolButton.TextOnly
    font.bold: true
    font.pointSize: Maui.Style.fontSizes.small

    // Display-only badge: unlike Index tabs, this does not open any overview.
    onClicked: {}

    background: Rectangle
    {
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
    }
}
