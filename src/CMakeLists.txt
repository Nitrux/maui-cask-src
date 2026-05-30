set(valenz_SRCS
    main.cpp
    controllers/valenzbridge.cpp)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${valenz_SRCS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(CMAKE_CONFIGURATION_TYPES)
    foreach(_cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${_cfg}" _cfg_upper)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${_cfg_upper} "${CMAKE_BINARY_DIR}/bin")
    endforeach()
endif()

qt_add_qml_module(${PROJECT_NAME}
    URI app.valenz
    VERSION 1.0
    NO_PLUGIN
    NO_IMPORT_SCAN
    RESOURCE_PREFIX /
    QML_FILES
    main.qml
    modules/ControlCenter.qml
    modules/WorkspaceBadge.qml
    modules/WorkspaceNavigation.qml
    modules/MprisControl.qml
    modules/WindowTitle.qml
    modules/WeatherClock.qml
    modules/CalendarPopup.qml
    modules/SystemTray.qml
    modules/ControlCenterButton.qml
    modules/NotificationsCenter.qml
    modules/NotificationsCenterButton.qml
    RESOURCES
    assets/cover.png)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MauiKit4
    MauiKit4::Calendar
    Qt6::Core
    Qt6::DBus
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2)

qt_finalize_target(${PROJECT_NAME})
