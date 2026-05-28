import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

import org.mauikit.controls as Maui

Dialog
{
    id: notificationsCenter

    property Item anchorButton
    property var rootWindow
    property Item overlayItem: notificationsCenter.parent
    property bool useSystemThemeIcons: true

    Maui.Theme.colorSet: Maui.Theme.View

    readonly property int _baseUnit: Math.max(20, Maui.Style.units.gridUnit)
    readonly property int _margin: Math.max(Maui.Style.contentMargins, Maui.Style.space.medium)
    readonly property int _dropOffset: 6
    readonly property int _panelInsetX: 8
    readonly property int _panelInsetY: 8
    readonly property color _panelColor: Maui.Theme.backgroundColor
    readonly property int _cardPadding: Math.max(Maui.Style.space.medium, Maui.Style.space.small + 2)
    readonly property int _minPanelWidth: Maui.Handy.isMobile ? _baseUnit * 16 : _baseUnit * 20
    readonly property int notificationCount: _notificationsModel.count
    property int _geometryRevision: 0

    readonly property real _targetY:
    {
        const overlayItem = notificationsCenter.overlayItem
        if (!overlayItem)
            return 0

        let targetY = Math.max(Maui.Style.toolBarHeightAlt, Maui.Style.units.gridUnit * 2) + _margin
        if (anchorButton)
        {
            const p = _anchorPointInOverlay(0, anchorButton.height)
            if (p)
                targetY = p.y + Maui.Style.space.small + _dropOffset
        }

        return targetY
    }

    readonly property real _availableHeightFromAnchor:
    {
        const overlayItem = notificationsCenter.overlayItem
        if (!overlayItem)
            return _panel.implicitHeight

        const minY = _margin
        const startY = Math.max(minY, _targetY)
        return Math.max(0, overlayItem.height - startY - _margin)
    }

    function _touchGeometryRevision()
    {
        _geometryRevision += 1
    }

    function _anchorPointInOverlay(offsetX, offsetY)
    {
        const overlayItem = notificationsCenter.parent
        if (!overlayItem || !anchorButton)
            return null

        if (anchorButton.mapToGlobal && overlayItem.mapFromGlobal)
        {
            const globalPoint = anchorButton.mapToGlobal(offsetX, offsetY)
            if (globalPoint && isFinite(globalPoint.x) && isFinite(globalPoint.y))
            {
                const localPoint = overlayItem.mapFromGlobal(globalPoint.x, globalPoint.y)
                if (localPoint && isFinite(localPoint.x) && isFinite(localPoint.y))
                    return localPoint
            }
        }

        const mappedPoint = anchorButton.mapToItem(overlayItem, offsetX, offsetY)
        if (mappedPoint && isFinite(mappedPoint.x) && isFinite(mappedPoint.y))
            return mappedPoint

        return null
    }

    function _notificationGlyph(iconName)
    {
        switch (String(iconName))
        {
            case "bluetooth-active": return "\uF294"
            case "battery": return "\uF240"
            case "settings-configure": return "\uF013"
            default: return "\uF0F3"
        }
    }

    function _urgencyAccentColor(urgencyLevel)
    {
        switch (urgencyLevel)
        {
            case 2: return Maui.Theme.negativeBackgroundColor
            case 1: return Maui.Theme.highlightColor
            case 0: return Qt.alpha(Maui.Theme.textColor, 0.28)
            default: return "transparent"
        }
    }

    function dismissNotification(index)
    {
        if (index >= 0 && index < _notificationsModel.count)
            _notificationsModel.remove(index, 1)
    }

    function clearNotifications()
    {
        _notificationsModel.clear()
    }

    ListModel
    {
        id: _notificationsModel

        ListElement
        {
            sourceName: "Power"
            messageText: "Power profile switched to Power Saver"
            timestampText: "Now"
            iconName: "battery"
            urgencyLevel: 1
            actionText: "Revert"
            actionKey: "power_revert"
        }

        ListElement
        {
            sourceName: "Bluetooth"
            messageText: "Headset connected"
            timestampText: "2 min ago"
            iconName: "bluetooth-active"
            urgencyLevel: 0
            actionText: "Disconnect"
            actionKey: "bt_disconnect"
        }

        ListElement
        {
            sourceName: "Software Update"
            messageText: "Security updates are ready to install."
            timestampText: "5 min ago"
            iconName: "system-software-update"
            urgencyLevel: 2
            actionText: "Install"
            actionKey: "updates_install"
        }

        ListElement
        {
            sourceName: "Calendar"
            messageText: "Meeting starts in 10 minutes."
            timestampText: "8 min ago"
            iconName: "office-calendar"
            urgencyLevel: 1
            actionText: "Open"
            actionKey: "calendar_open"
        }

        ListElement
        {
            sourceName: "Downloads"
            messageText: "Background sync completed successfully."
            timestampText: "12 min ago"
            iconName: "folder-download"
            urgencyLevel: 0
            actionText: "View"
            actionKey: "downloads_view"
        }
    }

    modal: false
    focus: true
    standardButtons: Dialog.NoButton
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 0
    transformOrigin: Item.TopRight

    enter: Transition
    {
        ParallelAnimation
        {
            NumberAnimation
            {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 170
                easing.type: Easing.OutCubic
            }

            NumberAnimation
            {
                property: "scale"
                from: 0.96
                to: 1.0
                duration: 170
                easing.type: Easing.OutCubic
            }
        }
    }

    exit: Transition
    {
        ParallelAnimation
        {
            NumberAnimation
            {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 130
                easing.type: Easing.OutCubic
            }

            NumberAnimation
            {
                property: "scale"
                from: 1.0
                to: 0.97
                duration: 130
                easing.type: Easing.OutCubic
            }
        }
    }

    width: Math.max(_panel.implicitWidth, _minPanelWidth)
    height: Math.min(_panel.implicitHeight, _availableHeightFromAnchor)

    onAboutToShow:
    {
        _touchGeometryRevision()
    }

    onOpened:
    {
        Qt.callLater(_touchGeometryRevision)
    }

    anchors.centerIn: undefined

    x:
    {
        const dep = _geometryRevision
        const overlayItem = notificationsCenter.overlayItem
        if (!overlayItem)
            return 0

        const minX = _margin
        const maxX = Math.max(minX, overlayItem.width - width - _margin)
        let targetX = maxX

        if (anchorButton)
        {
            const p = _anchorPointInOverlay(anchorButton.width, 0)
            if (p)
                targetX = p.x - width
        }

        return Math.max(minX, Math.min(maxX, targetX))
    }

    y:
    {
        const dep = _geometryRevision
        const overlayItem = notificationsCenter.overlayItem
        if (!overlayItem)
            return 0

        const minY = _margin
        return Math.max(minY, _targetY)
    }

    Connections
    {
        target: notificationsCenter.anchorButton

        function onXChanged() { notificationsCenter._touchGeometryRevision() }
        function onYChanged() { notificationsCenter._touchGeometryRevision() }
        function onWidthChanged() { notificationsCenter._touchGeometryRevision() }
        function onHeightChanged() { notificationsCenter._touchGeometryRevision() }
        function onVisibleChanged() { notificationsCenter._touchGeometryRevision() }
    }

    Connections
    {
        target: notificationsCenter.overlayItem

        function onWidthChanged() { notificationsCenter._touchGeometryRevision() }
        function onHeightChanged() { notificationsCenter._touchGeometryRevision() }
        function onXChanged() { notificationsCenter._touchGeometryRevision() }
        function onYChanged() { notificationsCenter._touchGeometryRevision() }
    }

    Connections
    {
        target: notificationsCenter.rootWindow

        function onWidthChanged() { notificationsCenter._touchGeometryRevision() }
        function onHeightChanged() { notificationsCenter._touchGeometryRevision() }
        function onVisibilityChanged() { notificationsCenter._touchGeometryRevision() }
        function onWindowStateChanged() { notificationsCenter._touchGeometryRevision() }
    }

    background: Rectangle
    {
        color: Qt.alpha(notificationsCenter._panelColor, 0)
    }

    contentItem: Rectangle
    {
        id: _panel
        implicitWidth: Math.max(notificationsCenter._minPanelWidth, _panelContent.implicitWidth + (notificationsCenter._panelInsetX * 2))
        implicitHeight: _panelContent.implicitHeight + (notificationsCenter._panelInsetY * 2)
        radius: Maui.Style.radiusV
        color: notificationsCenter._panelColor
        layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software
        layer.effect: MultiEffect
        {
            autoPaddingEnabled: true
            shadowEnabled: true
            shadowColor: "#80000000"
        }

        ColumnLayout
        {
            id: _panelContent
            anchors.fill: parent
            anchors.leftMargin: notificationsCenter._panelInsetX
            anchors.rightMargin: notificationsCenter._panelInsetX
            anchors.topMargin: notificationsCenter._panelInsetY
            anchors.bottomMargin: notificationsCenter._panelInsetY
            spacing: Maui.Style.space.small

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: false
                padding: notificationsCenter._cardPadding
                text: ""
                label2.text: ""

                RowLayout
                {
                    Layout.fillWidth: true
                    spacing: Maui.Style.space.small

                    ColumnLayout
                    {
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 2

                        Label
                        {
                            text: "Notifications"
                            color: Maui.Theme.textColor
                            font.weight: Font.DemiBold
                        }

                        Label
                        {
                            text: "Recent activity"
                            color: Qt.alpha(Maui.Theme.textColor, 0.85)
                        }
                    }

                    Item
                    {
                        Layout.fillWidth: true
                    }

                    Button
                    {
                        id: _clearAllButton
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        enabled: notificationsCenter.notificationCount > 0
                        onClicked: notificationsCenter.clearNotifications()

                        contentItem: RowLayout
                        {
                            spacing: Maui.Style.space.tiny

                            Item
                            {
                                Layout.alignment: Qt.AlignVCenter
                                width: 18
                                height: 18

                                Maui.Icon
                                {
                                    anchors.centerIn: parent
                                    width: 16
                                    height: 16
                                    source: "edit-clear-all"
                                    color: _clearAllButton.enabled ? Maui.Theme.textColor : Maui.Theme.disabledTextColor
                                    visible: notificationsCenter.useSystemThemeIcons
                                }

                                Label
                                {
                                    anchors.centerIn: parent
                                    visible: !notificationsCenter.useSystemThemeIcons
                                    text: "\uee23"
                                    color: _clearAllButton.enabled ? Maui.Theme.textColor : Maui.Theme.disabledTextColor
                                    font.family: "Symbols Nerd Font"
                                    font.weight: Font.Normal
                                    font.pixelSize: 12
                                    textFormat: Text.PlainText
                                    renderType: Text.QtRendering
                                }
                            }

                            Label
                            {
                                Layout.alignment: Qt.AlignVCenter
                                text: "Clear All"
                                color: _clearAllButton.enabled ? Maui.Theme.textColor : Maui.Theme.disabledTextColor
                            }
                        }
                    }
                }
            }

            Flickable
            {
                id: _notificationsFlick
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: _notificationsColumn.implicitHeight
                contentWidth: width
                contentHeight: _notificationsColumn.implicitHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                interactive: contentHeight > height

                Column
                {
                    id: _notificationsColumn
                    width: _notificationsFlick.width
                    spacing: Maui.Style.space.small

                    Maui.SectionItem
                    {
                        width: parent.width
                        visible: notificationsCenter.notificationCount === 0
                        flat: false
                        padding: notificationsCenter._cardPadding
                        text: "No notifications"
                        label2.text: "You are all caught up."
                    }

                    Repeater
                    {
                        model: _notificationsModel

                        delegate: Maui.SectionItem
                        {
                            required property int index
                            required property string sourceName
                            required property string messageText
                            required property string timestampText
                            required property string iconName
                            required property int urgencyLevel
                            required property string actionText
                            required property string actionKey

                            width: _notificationsColumn.width
                            flat: false
                            padding: notificationsCenter._cardPadding
                            text: ""
                            label2.text: ""
                            background: Rectangle
                            {
                                color: Maui.Theme.alternateBackgroundColor
                                radius: Maui.Style.radiusV
                                border.width: urgencyLevel >= 0 ? 1 : 0
                                border.color: notificationsCenter._urgencyAccentColor(urgencyLevel)
                            }

                            ColumnLayout
                            {
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small

                                RowLayout
                                {
                                    Layout.fillWidth: true
                                    spacing: Maui.Style.space.small

                                    Item
                                    {
                                        Layout.alignment: Qt.AlignVCenter
                                        width: 28
                                        height: 28

                                        Maui.Icon
                                        {
                                            anchors.centerIn: parent
                                            width: 22
                                            height: 22
                                            source: iconName
                                            color: Maui.Theme.textColor
                                            visible: notificationsCenter.useSystemThemeIcons
                                        }

                                        Label
                                        {
                                            anchors.centerIn: parent
                                            visible: !notificationsCenter.useSystemThemeIcons
                                            text: notificationsCenter._notificationGlyph(iconName)
                                            color: Maui.Theme.textColor
                                            font.family: "Symbols Nerd Font"
                                            font.weight: Font.Normal
                                            font.pixelSize: Math.max(12, Math.round(parent.height * 0.75))
                                            textFormat: Text.PlainText
                                            renderType: Text.QtRendering
                                        }
                                    }

                                    ColumnLayout
                                    {
                                        Layout.alignment: Qt.AlignVCenter
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Label
                                        {
                                            Layout.fillWidth: true
                                            text: sourceName
                                            color: Maui.Theme.textColor
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                        }
                                    }

                                    Label
                                    {
                                        Layout.alignment: Qt.AlignTop | Qt.AlignRight
                                        text: timestampText
                                        color: Qt.alpha(Maui.Theme.textColor, 0.7)
                                    }

                                    ToolButton
                                    {
                                        id: _dismissButton
                                        Layout.alignment: Qt.AlignTop | Qt.AlignRight
                                        display: ToolButton.IconOnly
                                        padding: 0
                                        implicitWidth: 16
                                        implicitHeight: 16
                                        icon.source: "qrc:/assets/close.svg"
                                        icon.width: 16
                                        icon.height: 16
                                        icon.color: hovered || down ? Maui.Theme.negativeTextColor : Maui.Theme.textColor
                                        background: Rectangle
                                        {
                                            radius: Maui.Style.radiusV
                                            color: _dismissButton.hovered || _dismissButton.down ? Maui.Theme.negativeBackgroundColor : "transparent"
                                        }
                                        onClicked: notificationsCenter.dismissNotification(index)
                                    }
                                }

                                Label
                                {
                                    Layout.fillWidth: true
                                    text: messageText
                                    color: Qt.alpha(Maui.Theme.textColor, 0.87)
                                    wrapMode: Text.WordWrap
                                }

                                RowLayout
                                {
                                    Layout.fillWidth: true

                                    Item
                                    {
                                        Layout.fillWidth: true
                                    }

                                    Button
                                    {
                                        text: actionText
                                        onClicked:
                                        {
                                            if (notificationsCenter.rootWindow && notificationsCenter.rootWindow.traceMenu)
                                                notificationsCenter.rootWindow.traceMenu("notification_action", actionKey)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
