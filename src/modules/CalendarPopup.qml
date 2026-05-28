import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

import org.mauikit.controls as Maui

Dialog
{
    id: calendarPopup

    property Item anchorItem
    property QtObject rootWindow
    property Item overlayItem: calendarPopup.parent
    property date selectedDate: new Date()
    property int displayMonth: selectedDate.getMonth()
    property int displayYear: selectedDate.getFullYear()
    property int reopenGuardMs: 180
    property double _lastClosedAtMs: -1
    property int _geometryRevision: 0
    readonly property int _baseUnit: Math.max(20, Maui.Style.units.gridUnit)
    readonly property int _margin: Math.max(Maui.Style.contentMargins, Maui.Style.space.medium)
    readonly property int _dropOffset: 6
    readonly property int _panelInsetX: 8
    readonly property int _panelInsetY: 8
    readonly property color _panelColor: Maui.Theme.backgroundColor
    readonly property int _preferredPanelWidth: Maui.Handy.isMobile ? _baseUnit * 16 : _baseUnit * 18
    readonly property int _preferredPanelHeight: Maui.Handy.isMobile ? _baseUnit * 15 : _baseUnit * 17

    Maui.Theme.colorSet: Maui.Theme.View

    function _touchGeometryRevision()
    {
        _geometryRevision += 1
    }

    function toggleFromAnchor()
    {
        if (visible)
        {
            close()
            return
        }

        if (_lastClosedAtMs >= 0 && (Date.now() - _lastClosedAtMs) < reopenGuardMs)
            return

        open()
    }

    function _anchorPointInOverlay(offsetX, offsetY)
    {
        const overlay = calendarPopup.overlayItem
        if (!overlay || !anchorItem)
            return null

        if (anchorItem.mapToGlobal && overlay.mapFromGlobal)
        {
            const globalPoint = anchorItem.mapToGlobal(offsetX, offsetY)
            if (globalPoint && isFinite(globalPoint.x) && isFinite(globalPoint.y))
            {
                const localPoint = overlay.mapFromGlobal(globalPoint.x, globalPoint.y)
                if (localPoint && isFinite(localPoint.x) && isFinite(localPoint.y))
                    return localPoint
            }
        }

        const mappedPoint = anchorItem.mapToItem(overlay, offsetX, offsetY)
        if (mappedPoint && isFinite(mappedPoint.x) && isFinite(mappedPoint.y))
            return mappedPoint

        return null
    }

    modal: false
    focus: true
    padding: 0
    standardButtons: Dialog.NoButton
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    transformOrigin: Item.Top

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

    width:
    {
        const overlay = calendarPopup.overlayItem
        if (!overlay)
            return _preferredPanelWidth

        const available = Math.max(0, overlay.width - (_margin * 2))
        if (available <= 0)
            return _preferredPanelWidth

        return Math.min(_preferredPanelWidth, available)
    }
    height:
    {
        const overlay = calendarPopup.overlayItem
        if (!overlay)
            return _preferredPanelHeight

        const available = Math.max(0, overlay.height - (_margin * 2))
        if (available <= 0)
            return _preferredPanelHeight

        return Math.min(_preferredPanelHeight, available)
    }
    onAboutToShow:
    {
        selectedDate = new Date()
        displayMonth = selectedDate.getMonth()
        displayYear = selectedDate.getFullYear()
        _touchGeometryRevision()
    }
    onOpened: Qt.callLater(_touchGeometryRevision)
    onClosed: _lastClosedAtMs = Date.now()
    anchors.centerIn: undefined
    x:
    {
        const dep = _geometryRevision
        const overlay = calendarPopup.overlayItem
        if (!overlay)
            return 0

        let targetX = _margin
        if (anchorItem)
        {
            const p = _anchorPointInOverlay(anchorItem.width / 2, anchorItem.height)
            if (p)
                targetX = p.x - (width / 2)
        }

        const minX = _margin
        const maxX = Math.max(minX, overlay.width - width - _margin)
        return Math.max(minX, Math.min(maxX, targetX))
    }
    y:
    {
        const dep = _geometryRevision
        const overlay = calendarPopup.overlayItem
        if (!overlay)
            return _margin

        if (!anchorItem)
            return _margin

        const minY = _margin
        const maxY = Math.max(minY, overlay.height - height - _margin)
        const p = _anchorPointInOverlay(0, anchorItem.height)
        if (p)
            return Math.max(minY, Math.min(maxY, p.y + Maui.Style.space.small + _dropOffset))

        return minY
    }

    Connections
    {
        target: calendarPopup.anchorItem

        function onXChanged() { calendarPopup._touchGeometryRevision() }
        function onYChanged() { calendarPopup._touchGeometryRevision() }
        function onWidthChanged() { calendarPopup._touchGeometryRevision() }
        function onHeightChanged() { calendarPopup._touchGeometryRevision() }
        function onVisibleChanged() { calendarPopup._touchGeometryRevision() }
    }

    Connections
    {
        target: calendarPopup.overlayItem

        function onWidthChanged() { calendarPopup._touchGeometryRevision() }
        function onHeightChanged() { calendarPopup._touchGeometryRevision() }
        function onXChanged() { calendarPopup._touchGeometryRevision() }
        function onYChanged() { calendarPopup._touchGeometryRevision() }
    }

    Connections
    {
        target: calendarPopup.rootWindow

        function onWidthChanged() { calendarPopup._touchGeometryRevision() }
        function onHeightChanged() { calendarPopup._touchGeometryRevision() }
        function onVisibilityChanged() { calendarPopup._touchGeometryRevision() }
        function onWindowStateChanged() { calendarPopup._touchGeometryRevision() }
    }

    background: Rectangle
    {
        color: Qt.alpha(calendarPopup._panelColor, 0)
    }

    contentItem: Rectangle
    {
        id: _panel
        implicitWidth: calendarPopup.width
        implicitHeight: calendarPopup.height
        radius: Maui.Style.radiusV
        color: calendarPopup._panelColor
        layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software
        layer.effect: MultiEffect
        {
            autoPaddingEnabled: true
            shadowEnabled: true
            shadowColor: "#80000000"
        }

        Item
        {
            id: _calendarContent
            anchors.fill: parent
            anchors.leftMargin: calendarPopup._panelInsetX
            anchors.rightMargin: calendarPopup._panelInsetX
            anchors.topMargin: calendarPopup._panelInsetY
            anchors.bottomMargin: calendarPopup._panelInsetY

            Maui.SectionItem
            {
                id: _calendarCard
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                flat: false
                padding: Maui.Style.space.medium
                text: ""
                label2.text: ""

                Item
                {
                    id: _headerRow
                    Layout.fillWidth: true
                    height: Math.max(Maui.Style.toolBarHeightAlt, _monthTitle.implicitHeight + Maui.Style.space.small)

                    ToolButton
                    {
                        id: _previousMonthButton
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        display: ToolButton.IconOnly
                        icon.name: "go-previous"
                        onClicked:
                        {
                            if (calendarPopup.displayMonth === 0)
                            {
                                calendarPopup.displayMonth = 11
                                calendarPopup.displayYear -= 1
                            }
                            else
                            {
                                calendarPopup.displayMonth -= 1
                            }
                        }
                    }

                    Label
                    {
                        id: _monthTitle
                        anchors.centerIn: parent
                        horizontalAlignment: Text.AlignHCenter
                        font.weight: Font.DemiBold
                        text:
                        {
                            const monthDate = new Date(calendarPopup.displayYear, calendarPopup.displayMonth, 1)
                            return Qt.formatDate(monthDate, "MMMM yyyy")
                        }
                    }

                    ToolButton
                    {
                        id: _nextMonthButton
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        display: ToolButton.IconOnly
                        icon.name: "go-next"
                        onClicked:
                        {
                            if (calendarPopup.displayMonth === 11)
                            {
                                calendarPopup.displayMonth = 0
                                calendarPopup.displayYear += 1
                            }
                            else
                            {
                                calendarPopup.displayMonth += 1
                            }
                        }
                    }
                }

                DayOfWeekRow
                {
                    id: _dayOfWeekRow
                    Layout.fillWidth: true
                    locale: Qt.locale()
                }

                MonthGrid
                {
                    id: _monthGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    month: calendarPopup.displayMonth
                    year: calendarPopup.displayYear
                    locale: Qt.locale()
                    onClicked:
                    {
                        calendarPopup.selectedDate = date
                    }
                }
            }
        }
    }
}
