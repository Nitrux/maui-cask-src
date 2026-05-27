import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Dialog
{
    id: controlCenter

    property Item anchorButton
    property var rootWindow
    property QtObject bridge
    property Item overlayItem: controlCenter.parent

    readonly property int _baseUnit: Math.max(20, Maui.Style.units.gridUnit)
    readonly property int _margin: Math.max(Maui.Style.contentMargins, Maui.Style.space.medium)
    readonly property int _dropOffset: 6
    readonly property color _cardColor: Qt.rgba(1, 1, 1, 0.28)
    readonly property color _titleColor: Qt.rgba(0.06, 0.09, 0.16, 0.94)
    readonly property color _subtitleColor: Qt.rgba(0.06, 0.09, 0.16, 0.72)
    property int _geometryRevision: 0

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

    modal: false
    focus: true
    standardButtons: Dialog.NoButton
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 0
    width: _panel.implicitWidth
    height: _panel.implicitHeight
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
        const maxY = Math.max(minY, overlayItem.height - height - _margin)
        let targetY = Math.max(Maui.Style.toolBarHeightAlt, Maui.Style.units.gridUnit * 2) + _margin

        if (anchorButton)
        {
            const p = _anchorPointInOverlay(0, anchorButton.height)
            if (p)
                targetY = p.y + Maui.Style.space.small + _dropOffset
        }

        return Math.max(minY, Math.min(maxY, targetY))
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
        color: "transparent"
    }

    contentItem: Rectangle
    {
        id: _panel
        implicitWidth: controlCenter._baseUnit * 18
        implicitHeight: _panelContent.implicitHeight + (Maui.Style.space.medium * 2)
        radius: Maui.Style.radiusV + 6
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.26)
        gradient: Gradient
        {
            GradientStop { position: 0.0; color: Qt.rgba(0.82, 0.91, 0.98, 0.94) }
            GradientStop { position: 1.0; color: Qt.rgba(0.62, 0.80, 0.97, 0.92) }
        }

        ColumnLayout
        {
            id: _panelContent
            anchors.fill: parent
            anchors.margins: Maui.Style.space.medium
            spacing: Maui.Style.space.medium

            RowLayout
            {
                Layout.fillWidth: true
                spacing: Maui.Style.space.small

                Rectangle
                {
                    Layout.fillWidth: true
                    implicitHeight: controlCenter._baseUnit * 1.5
                    radius: Maui.Style.radiusV + 4
                    color: controlCenter._cardColor

                    RowLayout
                    {
                        anchors.fill: parent
                        anchors.margins: Maui.Style.space.small
                        spacing: Maui.Style.space.small

                        Rectangle
                        {
                            width: Maui.Style.iconSizes.medium
                            height: width
                            radius: width / 2
                            color: Qt.rgba(1, 1, 1, 0.42)
                            clip: true

                            Image
                            {
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                source: "qrc:/app/valenz/assets/cover.png"
                            }
                        }

                        Label
                        {
                            Layout.fillWidth: true
                            text: "Hola Zaron"
                            font.weight: Font.DemiBold
                            color: controlCenter._titleColor
                            elide: Text.ElideRight
                        }
                    }
                }

                Rectangle
                {
                    Layout.preferredWidth: controlCenter._baseUnit * 5
                    implicitHeight: controlCenter._baseUnit * 1.5
                    radius: Maui.Style.radiusV + 4
                    color: controlCenter._cardColor

                    RowLayout
                    {
                        anchors.fill: parent
                        anchors.margins: Maui.Style.space.small
                        spacing: Maui.Style.space.small

                        Maui.Icon
                        {
                            source: "battery"
                            width: 16
                            height: 16
                            color: controlCenter._titleColor
                        }

                        Label
                        {
                            text: "62%"
                            color: controlCenter._titleColor
                        }

                        Item { Layout.fillWidth: true }

                        Maui.Icon
                        {
                            source: "system-shutdown"
                            width: 16
                            height: 16
                            color: Qt.rgba(0.86, 0.22, 0.26, 1.0)
                        }
                    }
                }
            }

            RowLayout
            {
                Layout.fillWidth: true
                spacing: Maui.Style.space.small

                Rectangle
                {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 2
                    implicitHeight: controlCenter._baseUnit * 7
                    radius: Maui.Style.radiusV + 4
                    color: controlCenter._cardColor

                    ColumnLayout
                    {
                        anchors.fill: parent
                        anchors.margins: Maui.Style.space.small
                        spacing: Maui.Style.space.small

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Rectangle
                            {
                                width: Maui.Style.iconSizes.medium
                                height: width
                                radius: width / 2
                                color: Qt.rgba(0.12, 0.42, 0.92, 0.95)
                                Maui.Icon { anchors.centerIn: parent; source: "network-wireless"; width: 16; height: 16; color: "white" }
                            }

                            Label
                            {
                                text: "Red"
                                font.weight: Font.DemiBold
                                color: controlCenter._titleColor
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Rectangle
                            {
                                width: Maui.Style.iconSizes.medium
                                height: width
                                radius: width / 2
                                color: Qt.rgba(0.12, 0.42, 0.92, 0.95)
                                Maui.Icon { anchors.centerIn: parent; source: "bluetooth-active"; width: 16; height: 16; color: "white" }
                            }

                            ColumnLayout
                            {
                                Layout.fillWidth: true
                                spacing: 0
                                Label { text: "Bluetooth"; font.weight: Font.DemiBold; color: controlCenter._titleColor }
                                Label { text: "No conectado"; color: controlCenter._subtitleColor }
                            }
                        }

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: Maui.Style.space.small

                            Rectangle
                            {
                                width: Maui.Style.iconSizes.medium
                                height: width
                                radius: width / 2
                                color: Qt.rgba(0.12, 0.42, 0.92, 0.95)
                                Maui.Icon { anchors.centerIn: parent; source: "settings-configure"; width: 16; height: 16; color: "white" }
                            }

                            ColumnLayout
                            {
                                Layout.fillWidth: true
                                spacing: 0
                                Label { text: "Configuracion"; font.weight: Font.DemiBold; color: controlCenter._titleColor }
                                Label
                                {
                                    text: "Configuracion del sis..."
                                    color: controlCenter._subtitleColor
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                ColumnLayout
                {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 2
                    spacing: Maui.Style.space.small

                    Rectangle
                    {
                        Layout.fillWidth: true
                        implicitHeight: controlCenter._baseUnit * 3.6
                        radius: Maui.Style.radiusV + 4
                        color: controlCenter._cardColor

                        ColumnLayout
                        {
                            anchors.fill: parent
                            anchors.margins: Maui.Style.space.small
                            spacing: Maui.Style.space.small

                            Label
                            {
                                text: "Volumen"
                                font.weight: Font.DemiBold
                                color: controlCenter._titleColor
                            }

                            RowLayout
                            {
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small

                                Slider
                                {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 100
                                    value: 62
                                }

                                Rectangle
                                {
                                    width: Maui.Style.iconSizes.medium
                                    height: width
                                    radius: width / 2
                                    color: Qt.rgba(0.45, 0.50, 0.62, 0.36)
                                    Maui.Icon { anchors.centerIn: parent; source: "configure"; width: 16; height: 16; color: controlCenter._titleColor }
                                }
                            }
                        }
                    }

                    RowLayout
                    {
                        Layout.fillWidth: true
                        spacing: Maui.Style.space.small

                        Rectangle
                        {
                            Layout.fillWidth: true
                            implicitHeight: controlCenter._baseUnit * 2.9
                            radius: Maui.Style.radiusV + 4
                            color: controlCenter._cardColor

                            Column
                            {
                                anchors.centerIn: parent
                                spacing: Maui.Style.space.tiny
                                Maui.Icon { anchors.horizontalCenter: parent.horizontalCenter; source: "weather-clear-night"; width: 16; height: 16; color: controlCenter._titleColor }
                                Label { text: "Off"; color: controlCenter._titleColor; horizontalAlignment: Text.AlignHCenter }
                            }
                        }

                        Rectangle
                        {
                            Layout.fillWidth: true
                            implicitHeight: controlCenter._baseUnit * 2.9
                            radius: Maui.Style.radiusV + 4
                            color: controlCenter._cardColor

                            Column
                            {
                                anchors.centerIn: parent
                                spacing: Maui.Style.space.tiny
                                Maui.Icon { anchors.horizontalCenter: parent.horizontalCenter; source: "notifications-disabled"; width: 16; height: 16; color: controlCenter._titleColor }
                                Label { text: "DND"; color: controlCenter._titleColor; horizontalAlignment: Text.AlignHCenter }
                            }
                        }
                    }
                }
            }

            Rectangle
            {
                Layout.fillWidth: true
                implicitHeight: controlCenter._baseUnit * 2.7
                radius: Maui.Style.radiusV + 4
                color: controlCenter._cardColor

                RowLayout
                {
                    anchors.fill: parent
                    anchors.margins: Maui.Style.space.small
                    spacing: Maui.Style.space.medium

                    Maui.Icon
                    {
                        source: "weather-few-clouds"
                        width: Maui.Style.iconSizes.medium
                        height: width
                        color: Qt.rgba(0.96, 0.68, 0.18, 1.0)
                    }

                    RowLayout
                    {
                        spacing: Maui.Style.space.tiny

                        Label
                        {
                            text: "25°"
                            color: controlCenter._titleColor
                            font.pointSize: Maui.Style.fontSizes.big
                        }

                        Label
                        {
                            text: "15.6°"
                            color: controlCenter._titleColor
                            font.weight: Font.Bold
                            font.pixelSize: 35
                        }
                    }

                    Item { Layout.fillWidth: true }

                    ColumnLayout
                    {
                        spacing: 0
                        Label { text: "Despejado"; color: controlCenter._titleColor; font.pointSize: Maui.Style.fontSizes.medium }
                        Label { text: "Durango"; color: controlCenter._subtitleColor }
                    }
                }
            }

            Rectangle
            {
                Layout.fillWidth: true
                implicitHeight: controlCenter._baseUnit * 2.8
                radius: Maui.Style.radiusV + 4
                color: controlCenter._cardColor

                RowLayout
                {
                    anchors.fill: parent
                    anchors.margins: Maui.Style.space.small
                    spacing: Maui.Style.space.small

                    Rectangle
                    {
                        width: controlCenter._baseUnit * 2.1
                        height: width
                        radius: Maui.Style.radiusV
                        clip: true
                        color: Qt.rgba(0.1, 0.1, 0.2, 0.24)

                        Image
                        {
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                            source: "qrc:/app/valenz/assets/cover.png"
                        }
                    }

                    ColumnLayout
                    {
                        Layout.fillWidth: true
                        spacing: 0
                        Label
                        {
                            text: "Mi Heroe"
                            font.weight: Font.DemiBold
                            color: controlCenter._titleColor
                            elide: Text.ElideRight
                        }
                        Label
                        {
                            text: "Antonio Orozco"
                            color: controlCenter._subtitleColor
                            elide: Text.ElideRight
                        }
                    }

                    RowLayout
                    {
                        spacing: Maui.Style.space.small
                        ToolButton { icon.name: "media-playback-pause"; display: ToolButton.IconOnly }
                        ToolButton { icon.name: "media-skip-forward"; display: ToolButton.IconOnly }
                    }
                }
            }
        }
    }
}
