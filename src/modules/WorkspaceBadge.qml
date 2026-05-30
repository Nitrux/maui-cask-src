import QtQuick
import QtQuick.Controls

import org.mauikit.controls as Maui

ToolButton
{
    id: workspaceBadge

    property QtObject bridge
    property string badgeText: ""

    text: badgeText.length > 0 ? badgeText : (workspaceBadge.bridge ? (String(workspaceBadge.bridge.currentWorkspace) + "/" + String(Math.max(1, workspaceBadge.bridge.workspaceCount))) : "1/1")
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
