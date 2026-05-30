import QtQuick
import QtQuick.Controls

import org.mauikit.controls as Maui

Maui.ToolActions
{
    id: workspaceNavigation

    property QtObject bridge

    display: ToolButton.IconOnly
    checkable: false
    autoExclusive: false

    Action
    {
        text: "Previous workspace"
        icon.name: "go-previous"
        enabled: !!workspaceNavigation.bridge
        onTriggered:
        {
            workspaceNavigation.bridge.goToPreviousWorkspace()
        }
    }

    Action
    {
        text: "Next workspace"
        icon.name: "go-next"
        enabled: !!workspaceNavigation.bridge
        onTriggered:
        {
            workspaceNavigation.bridge.goToNextWorkspace()
        }
    }
}
