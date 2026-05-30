import QtQuick
import QtQuick.Controls
import QtQuick.Controls 2.15 as QQC
import QtQuick.Layouts
import QtQuick.Effects

import org.mauikit.controls as Maui

Dialog
{
    id: controlCenter

    property Item anchorButton
    property var rootWindow
    property QtObject bridge
    property Item overlayItem: controlCenter.parent

    Maui.Theme.colorSet: Maui.Theme.View

    readonly property int _baseUnit: Math.max(20, Maui.Style.units.gridUnit)
    readonly property int _margin: Math.max(Maui.Style.contentMargins, Maui.Style.space.medium)
    readonly property int _dropOffset: 6
    readonly property int _panelInsetX: 8
    readonly property int _panelInsetY: 8
    // readonly property color _panelColor: Qt.alpha(Maui.Theme.backgroundColor, 0.76)
    readonly property color _panelColor: Maui.Theme.backgroundColor
    readonly property string _nerdFontFamily: "Symbols Nerd Font Mono"
    readonly property int _glyphSize: Math.max(15, Math.round(Maui.Style.iconSizes.medium * 0.9))
    readonly property int _cardPadding: Maui.Style.space.small
    readonly property int _toolActionMinSize: Math.max(28, Maui.Style.toolBarHeightAlt)
    readonly property int _minPanelWidth: Maui.Handy.isMobile ? _baseUnit * 16 : _baseUnit * 20
    property bool _cellularEnabled: false
    property bool _dndEnabled: false
    property bool _nightLightEnabled: true
    property int _geometryRevision: 0
    readonly property real _targetY:
    {
        const overlayItem = controlCenter.overlayItem
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
        const overlayItem = controlCenter.overlayItem
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
        const overlayItem = controlCenter.parent
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

    function _powerProfileLabel(profileName)
    {
        switch (profileName)
        {
            case "power-saver": return "Power Saver"
            case "balanced": return "Balanced"
            case "performance": return "Performance"
            default: return profileName
        }
    }

    function _networkModeTitle()
    {
        const state = controlCenter.bridge ? String(controlCenter.bridge.prototypeNetworkState).toLowerCase() : "wireless"
        return state === "wired" ? "Wired" : "WiFi"
    }

    function _networkModeSubtitle()
    {
        const state = controlCenter.bridge ? String(controlCenter.bridge.prototypeNetworkState).toLowerCase() : "wireless"
        if (state === "offline")
            return "Off"
        if (state === "wired")
            return "Connected"
        if (state === "hotspot")
            return "Hotspot"
        if (state === "vpn")
            return "VPN"
        if (state === "cellular")
            return "Cellular"
        return "Home_Network"
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
        const overlayItem = controlCenter.overlayItem
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
        const overlayItem = controlCenter.overlayItem
        if (!overlayItem)
            return 0

        const minY = _margin
        return Math.max(minY, _targetY)
    }

    Connections
    {
        target: controlCenter.anchorButton

        function onXChanged() { controlCenter._touchGeometryRevision() }
        function onYChanged() { controlCenter._touchGeometryRevision() }
        function onWidthChanged() { controlCenter._touchGeometryRevision() }
        function onHeightChanged() { controlCenter._touchGeometryRevision() }
        function onVisibleChanged() { controlCenter._touchGeometryRevision() }
    }

    Connections
    {
        target: controlCenter.overlayItem

        function onWidthChanged() { controlCenter._touchGeometryRevision() }
        function onHeightChanged() { controlCenter._touchGeometryRevision() }
        function onXChanged() { controlCenter._touchGeometryRevision() }
        function onYChanged() { controlCenter._touchGeometryRevision() }
    }

    Connections
    {
        target: controlCenter.rootWindow

        function onWidthChanged() { controlCenter._touchGeometryRevision() }
        function onHeightChanged() { controlCenter._touchGeometryRevision() }
        function onVisibilityChanged() { controlCenter._touchGeometryRevision() }
        function onWindowStateChanged() { controlCenter._touchGeometryRevision() }
    }

    background: Rectangle
    {
        color: Qt.alpha(controlCenter._panelColor, 0)
    }

    contentItem: Rectangle
    {
        id: _panel
        implicitWidth: Math.max(controlCenter._minPanelWidth, _panelContent.implicitWidth + (controlCenter._panelInsetX * 2))
        implicitHeight: _panelContent.implicitHeight + (controlCenter._panelInsetY * 2)
        radius: Maui.Style.radiusV
        color: controlCenter._panelColor
        layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software
        layer.effect: MultiEffect
        {
            autoPaddingEnabled: true
            shadowEnabled: true
            shadowColor: "#80000000"
        }

        Flickable
        {
            id: _panelFlick
            anchors.fill: parent
            anchors.leftMargin: controlCenter._panelInsetX
            anchors.rightMargin: controlCenter._panelInsetX
            anchors.topMargin: controlCenter._panelInsetY
            anchors.bottomMargin: controlCenter._panelInsetY
            contentWidth: width
            contentHeight: _panelContent.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: contentHeight > height

            ColumnLayout
        {
            id: _panelContent
            width: _panelFlick.width
            spacing: Maui.Style.space.small

            GridLayout
            {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: Maui.Style.space.small
                columnSpacing: Maui.Style.space.small

                Maui.SectionItem
                {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Rectangle
                            {
                                implicitWidth: Maui.Style.iconSizes.big + Maui.Style.space.small
                                implicitHeight: implicitWidth
                                radius: implicitWidth / 2
                                color: Maui.Theme.alternateBackgroundColor

                                Label
                                {
                                    anchors.centerIn: parent
                                    text: "\uf007"
                                    font.family: controlCenter._nerdFontFamily
                                    font.pixelSize: controlCenter._glyphSize
                                    color: Maui.Theme.textColor
                                }
                            }

                            Label
                            {
                                text: "Alex R."
                                color: Maui.Theme.textColor
                                font.weight: Font.DemiBold
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item { Layout.fillWidth: true }

                            ToolButton
                            {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.minimumHeight: 28
                                text: "Power"
                                icon.name: "system-shutdown"
                                display: ToolButton.TextBesideIcon

                                background: Rectangle
                                {
                                    radius: Math.round(Maui.Style.radiusV * 0.6)
                                    color: parent.down ? "#cc3b49" : "#e34b5a"
                                }

                                onClicked:
                                {
                                    if (controlCenter.bridge)
                                        controlCenter.bridge.trace("controlCenter", "shortcut_power")
                                }
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Label
                            {
                                text: "\uf1eb"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }

                            ColumnLayout
                            {
                                spacing: 0
                                Label { text: controlCenter._networkModeTitle(); color: Maui.Theme.textColor; font.weight: Font.DemiBold }
                                Label { text: controlCenter._networkModeSubtitle(); color: Maui.Theme.disabledTextColor }
                            }

                            Item { Layout.fillWidth: true }

                            Switch
                            {
                                checked: controlCenter.bridge ? controlCenter.bridge.prototypeNetworkState !== "offline" : true
                                onToggled:
                                {
                                    if (!controlCenter.bridge)
                                        return

                                    if (checked)
                                    {
                                        const state = String(controlCenter.bridge.prototypeNetworkState).toLowerCase()
                                        controlCenter.bridge.prototypeNetworkState = state === "wired" ? "wired" : "wireless"
                                    }
                                    else
                                    {
                                        controlCenter.bridge.prototypeNetworkState = "offline"
                                    }
                                }
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Label
                            {
                                text: "\uf294"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }

                            ColumnLayout
                            {
                                spacing: 0

                                Label
                                {
                                    text: "Bluetooth"
                                    color: Maui.Theme.textColor
                                    font.weight: Font.DemiBold
                                }
                                Label
                                {
                                    text: controlCenter.bridge && controlCenter.bridge.prototypeBluetoothState === "off" ? "Off" : "On"
                                    color: Maui.Theme.disabledTextColor
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Switch
                            {
                                checked: controlCenter.bridge ? controlCenter.bridge.prototypeBluetoothState !== "off" : true
                                onToggled:
                                {
                                    if (controlCenter.bridge)
                                        controlCenter.bridge.prototypeBluetoothState = checked ? "on" : "off"
                                }
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Label
                            {
                                text: "\uf186"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }

                            ColumnLayout
                            {
                                spacing: 0
                                Label { text: "Night Light"; color: Maui.Theme.textColor; font.weight: Font.DemiBold }
                                Label { text: controlCenter._nightLightEnabled ? "On" : "Off"; color: Maui.Theme.disabledTextColor }
                            }

                            Item { Layout.fillWidth: true }

                            Switch
                            {
                                checked: controlCenter._nightLightEnabled
                                onToggled: controlCenter._nightLightEnabled = checked
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Label
                            {
                                text: "\uf05e"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }

                            ColumnLayout
                            {
                                spacing: 0
                                Label { text: "Do Not Disturb"; color: Maui.Theme.textColor; font.weight: Font.DemiBold }
                                Label { text: controlCenter._dndEnabled ? "On" : "Off"; color: Maui.Theme.disabledTextColor }
                            }

                            Item { Layout.fillWidth: true }

                            Switch
                            {
                                checked: controlCenter._dndEnabled
                                onToggled: controlCenter._dndEnabled = checked
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Label
                            {
                                text: "\uf0e7"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }

                            ColumnLayout
                            {
                                Layout.fillWidth: true
                                spacing: 0

                                Label
                                {
                                    text: "Power Profile"
                                    color: Maui.Theme.textColor
                                    font.weight: Font.DemiBold
                                }

                                Label
                                {
                                    text: controlCenter._powerProfileLabel(controlCenter.bridge ? controlCenter.bridge.controlCenterPowerProfileCurrent : "balanced")
                                    color: Maui.Theme.disabledTextColor
                                }
                            }

                            Item { Layout.fillWidth: true }

                            QQC.ComboBox
                            {
                                id: _powerProfileSelector
                                readonly property string _fallbackLabel: "Select Profile"
                                readonly property string _longestLabel:
                                {
                                    const entries = controlCenter.bridge ? controlCenter.bridge.controlCenterPowerProfiles : []
                                    let longest = _fallbackLabel

                                    for (let i = 0; i < entries.length; ++i)
                                    {
                                        const candidate = controlCenter._powerProfileLabel(entries[i])
                                        if (candidate.length > longest.length)
                                            longest = candidate
                                    }

                                    return longest
                                }
                                readonly property real _contentDrivenWidth: Math.ceil(_powerProfileLongestLabelMetrics.advanceWidth + (Maui.Style.space.big * 3) + Maui.Style.iconSizes.small)
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredWidth: _contentDrivenWidth
                                Layout.minimumWidth: _contentDrivenWidth
                                Layout.maximumWidth: _contentDrivenWidth
                                currentIndex: -1
                                enabled: controlCenter.bridge && controlCenter.bridge.controlCenterPowerProfiles.length > 0
                                model: controlCenter.bridge ? controlCenter.bridge.controlCenterPowerProfiles : []
                                displayText: currentIndex === -1 ? _fallbackLabel : controlCenter._powerProfileLabel(currentText)
                                TextMetrics
                                {
                                    id: _powerProfileLongestLabelMetrics
                                    text: _powerProfileSelector._longestLabel
                                    font: _powerProfileSelector.font
                                }
                                delegate: ItemDelegate
                                {
                                    required property string modelData
                                    width: ListView.view ? ListView.view.width : _powerProfileSelector.width
                                    text: controlCenter._powerProfileLabel(modelData)
                                }
                                popup.width:
                                {
                                    const popupContentWidth = popup.contentItem ? popup.contentItem.implicitWidth : 0
                                    const popupFrameWidth = popup.leftPadding + popup.rightPadding
                                    return Math.max(_powerProfileSelector.width, popupContentWidth + popupFrameWidth)
                                }
                                Component.onCompleted:
                                {
                                    if (!controlCenter.bridge)
                                        return

                                    const idx = model.indexOf(controlCenter.bridge.controlCenterPowerProfileCurrent)
                                    currentIndex = idx
                                }
                                Connections
                                {
                                    target: controlCenter.bridge
                                    enabled: !!controlCenter.bridge

                                    function onControlCenterPowerProfilesChanged()
                                    {
                                        if (!controlCenter.bridge)
                                            return

                                        const idx = _powerProfileSelector.model.indexOf(controlCenter.bridge.controlCenterPowerProfileCurrent)
                                        _powerProfileSelector.currentIndex = idx
                                    }

                                    function onControlCenterPowerProfileCurrentChanged()
                                    {
                                        if (!controlCenter.bridge)
                                            return

                                        const idx = _powerProfileSelector.model.indexOf(controlCenter.bridge.controlCenterPowerProfileCurrent)
                                        _powerProfileSelector.currentIndex = idx
                                    }
                                }
                                onActivated:
                                {
                                    if (currentIndex >= 0)
                                    {
                                        if (controlCenter.bridge)
                                            controlCenter.bridge.controlCenterPowerProfileCurrent = currentText
                                    }
                                }
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small
                            Label
                            {
                                text: "\uf026"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }
                            Slider
                            {
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: 62
                            }
                            Label
                            {
                                text: "\uf028"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small
                            Label
                            {
                                text: "\uf042"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }
                            Slider
                            {
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: 70
                            }
                            Label
                            {
                                text: "\uf185"
                                font.family: controlCenter._nerdFontFamily
                                font.pixelSize: controlCenter._glyphSize
                                color: Maui.Theme.textColor
                            }
                        }
                    }

                    Maui.SectionItem
                    {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        flat: false
                        padding: controlCenter._cardPadding
                        text: ""
                        label2.text: ""

                        ColumnLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            RowLayout
                            {
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small
                                Label { text: "\uf2db"; font.family: controlCenter._nerdFontFamily; font.pixelSize: controlCenter._glyphSize; color: Maui.Theme.textColor }
                                Label { text: "CPU"; color: Maui.Theme.textColor; font.weight: Font.DemiBold }
                                Label { text: "25%"; color: Maui.Theme.disabledTextColor }
                            }
                            ProgressBar
                            {
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: 25
                            }

                            RowLayout
                            {
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small
                                Label { text: "\uefc5"; font.family: controlCenter._nerdFontFamily; font.pixelSize: controlCenter._glyphSize; color: Maui.Theme.textColor }
                                Label { text: "RAM"; color: Maui.Theme.textColor; font.weight: Font.DemiBold }
                                Label { text: "60%"; color: Maui.Theme.disabledTextColor }
                            }
                            ProgressBar
                            {
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: 60
                            }

                            RowLayout
                            {
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small
                                Label { text: "\uf0a0"; font.family: controlCenter._nerdFontFamily; font.pixelSize: controlCenter._glyphSize; color: Maui.Theme.textColor }
                                Label { text: "Disk"; color: Maui.Theme.textColor; font.weight: Font.DemiBold }
                                Label { text: "512GB / 1TB"; color: Maui.Theme.disabledTextColor }
                            }
                            ProgressBar
                            {
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: 50
                            }
                        }
                }
            }
        }
        }
    }
}
