import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Item
{
    id: weatherClock

    property string clockText: ""
    property string dateText: ""
    property string weatherIconName: "weather-clear"
    property string weatherTemperature: ""

    implicitWidth: _clockWeatherGrid.implicitWidth
    implicitHeight: _clockWeatherGrid.implicitHeight

    GridLayout
    {
        id: _clockWeatherGrid
        anchors.centerIn: parent
        columns: 2
        columnSpacing: Maui.Style.space.small
        rowSpacing: 0

        Label
        {
            Layout.row: 0
            Layout.column: 0
            text: weatherClock.clockText
            font.weight: Font.DemiBold
            font.pointSize: Maui.Style.fontSizes.big
            horizontalAlignment: Text.AlignLeft
        }

        Label
        {
            Layout.row: 1
            Layout.column: 0
            text: weatherClock.dateText
            color: Maui.Theme.disabledTextColor
            font.pointSize: Maui.Style.fontSizes.tiny
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout
        {
            Layout.row: 0
            Layout.column: 1
            Layout.rowSpan: 2
            Layout.alignment: Qt.AlignVCenter
            spacing: Maui.Style.space.tiny

            Maui.Icon
            {
                Layout.alignment: Qt.AlignVCenter
                width: Maui.Style.iconSizes.small
                height: width
                source: weatherClock.weatherIconName
                color: Maui.Theme.textColor
            }

            Label
            {
                Layout.alignment: Qt.AlignVCenter
                text: weatherClock.weatherTemperature
                font.weight: Font.DemiBold
                font.pointSize: Maui.Style.fontSizes.big
            }
        }
    }
}
