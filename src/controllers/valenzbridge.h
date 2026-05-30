#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QVariantMap>

class QLocalSocket;
class QTimer;

class ValenzBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(int currentWorkspace READ currentWorkspace WRITE setCurrentWorkspace NOTIFY currentWorkspaceChanged FINAL)
    Q_PROPERTY(int workspaceCount READ workspaceCount WRITE setWorkspaceCount NOTIFY workspaceCountChanged FINAL)
    Q_PROPERTY(QString mediaTitle READ mediaTitle WRITE setMediaTitle NOTIFY mediaTitleChanged FINAL)
    Q_PROPERTY(QString mediaArtist READ mediaArtist WRITE setMediaArtist NOTIFY mediaArtistChanged FINAL)
    Q_PROPERTY(QString mediaTimestamp READ mediaTimestamp WRITE setMediaTimestamp NOTIFY mediaTimestampChanged FINAL)
    Q_PROPERTY(QString mediaArtSource READ mediaArtSource WRITE setMediaArtSource NOTIFY mediaArtSourceChanged FINAL)
    Q_PROPERTY(bool mediaPlaying READ mediaPlaying WRITE setMediaPlaying NOTIFY mediaPlayingChanged FINAL)
    Q_PROPERTY(bool mprisVisible READ mprisVisible WRITE setMprisVisible NOTIFY mprisVisibleChanged FINAL)
    Q_PROPERTY(QString focusedWindowTitle READ focusedWindowTitle WRITE setFocusedWindowTitle NOTIFY focusedWindowTitleChanged FINAL)
    Q_PROPERTY(QString focusedWindowIconName READ focusedWindowIconName WRITE setFocusedWindowIconName NOTIFY focusedWindowIconNameChanged FINAL)
    Q_PROPERTY(QString controlCenterIconMode READ controlCenterIconMode WRITE setControlCenterIconMode NOTIFY controlCenterIconModeChanged FINAL)
    Q_PROPERTY(QString prototypeNetworkState READ prototypeNetworkState WRITE setPrototypeNetworkState NOTIFY prototypeNetworkStateChanged FINAL)
    Q_PROPERTY(QString prototypeBluetoothState READ prototypeBluetoothState WRITE setPrototypeBluetoothState NOTIFY prototypeBluetoothStateChanged FINAL)
    Q_PROPERTY(QString prototypeVolumeState READ prototypeVolumeState WRITE setPrototypeVolumeState NOTIFY prototypeVolumeStateChanged FINAL)
    Q_PROPERTY(QStringList controlCenterPowerProfiles READ controlCenterPowerProfiles WRITE setControlCenterPowerProfiles NOTIFY controlCenterPowerProfilesChanged FINAL)
    Q_PROPERTY(QString controlCenterPowerProfileCurrent READ controlCenterPowerProfileCurrent WRITE setControlCenterPowerProfileCurrent NOTIFY controlCenterPowerProfileCurrentChanged FINAL)
    Q_PROPERTY(QString controlCenterVolumePercentage READ controlCenterVolumePercentage WRITE setControlCenterVolumePercentage NOTIFY controlCenterVolumePercentageChanged FINAL)
    Q_PROPERTY(bool controlCenterBatteryCharging READ controlCenterBatteryCharging WRITE setControlCenterBatteryCharging NOTIFY controlCenterBatteryChargingChanged FINAL)
    Q_PROPERTY(QString controlCenterBatteryPercentage READ controlCenterBatteryPercentage WRITE setControlCenterBatteryPercentage NOTIFY controlCenterBatteryPercentageChanged FINAL)

public:
    explicit ValenzBridge(QObject *parent = nullptr);

    bool enabled() const;
    void setEnabled(bool enabled);
    int currentWorkspace() const;
    void setCurrentWorkspace(int workspace);
    int workspaceCount() const;
    void setWorkspaceCount(int count);
    QString mediaTitle() const;
    void setMediaTitle(const QString &title);
    QString mediaArtist() const;
    void setMediaArtist(const QString &artist);
    QString mediaTimestamp() const;
    void setMediaTimestamp(const QString &timestamp);
    QString mediaArtSource() const;
    void setMediaArtSource(const QString &source);
    bool mediaPlaying() const;
    void setMediaPlaying(bool playing);
    bool mprisVisible() const;
    void setMprisVisible(bool visible);
    QString focusedWindowTitle() const;
    void setFocusedWindowTitle(const QString &title);
    QString focusedWindowIconName() const;
    void setFocusedWindowIconName(const QString &iconName);
    QString controlCenterIconMode() const;
    void setControlCenterIconMode(const QString &mode);
    QString prototypeNetworkState() const;
    void setPrototypeNetworkState(const QString &state);
    QString prototypeBluetoothState() const;
    void setPrototypeBluetoothState(const QString &state);
    QString prototypeVolumeState() const;
    void setPrototypeVolumeState(const QString &state);
    QStringList controlCenterPowerProfiles() const;
    void setControlCenterPowerProfiles(const QStringList &profiles);
    QString controlCenterPowerProfileCurrent() const;
    void setControlCenterPowerProfileCurrent(const QString &profile);
    QString controlCenterVolumePercentage() const;
    void setControlCenterVolumePercentage(const QString &value);
    bool controlCenterBatteryCharging() const;
    void setControlCenterBatteryCharging(bool charging);
    QString controlCenterBatteryPercentage() const;
    void setControlCenterBatteryPercentage(const QString &value);

    Q_INVOKABLE void trace(const QString &source, const QString &action, const QString &detail = QString());
    Q_INVOKABLE void goToPreviousWorkspace();
    Q_INVOKABLE void goToNextWorkspace();
    Q_INVOKABLE bool refreshWorkspaceState();
    Q_INVOKABLE void mediaPreviousTrack();
    Q_INVOKABLE void mediaTogglePlayPause();
    Q_INVOKABLE void mediaNextTrack();
    Q_INVOKABLE QString configFilePath() const;

Q_SIGNALS:
    void traceRaised(const QString &source, const QString &action, const QString &detail, const QString &timestamp);
    void enabledChanged(bool enabled);
    void currentWorkspaceChanged(int currentWorkspace);
    void workspaceCountChanged(int workspaceCount);
    void mediaTitleChanged(const QString &title);
    void mediaArtistChanged(const QString &artist);
    void mediaTimestampChanged(const QString &timestamp);
    void mediaArtSourceChanged(const QString &source);
    void mediaPlayingChanged(bool playing);
    void mprisVisibleChanged(bool visible);
    void focusedWindowTitleChanged(const QString &title);
    void focusedWindowIconNameChanged(const QString &iconName);
    void controlCenterIconModeChanged(const QString &mode);
    void prototypeNetworkStateChanged(const QString &state);
    void prototypeBluetoothStateChanged(const QString &state);
    void prototypeVolumeStateChanged(const QString &state);
    void controlCenterPowerProfilesChanged(const QStringList &profiles);
    void controlCenterPowerProfileCurrentChanged(const QString &profile);
    void controlCenterVolumePercentageChanged(const QString &value);
    void controlCenterBatteryChargingChanged(bool charging);
    void controlCenterBatteryPercentageChanged(const QString &value);

private Q_SLOTS:
    void onMprisPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void onMprisNameOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);

private:
    int clampWorkspace(int workspace) const;
    void initializeConfig();
    void connectHyprlandEventSocket();
    void scheduleHyprlandEventSocketReconnect();
    void handleHyprlandEventData();
    void handleHyprlandEventLine(const QString &line);
    bool refreshFocusedWindowState();
    QStringList mprisServiceNames() const;
    QString preferredMprisService() const;
    QVariantMap mprisPlayerProperties(const QString &serviceName) const;
    QVariantMap mprisPlayerMetadata(const QString &serviceName) const;
    qint64 mprisPlayerPositionUs(const QString &serviceName) const;
    QString formatMprisTimeUs(qint64 microseconds) const;
    QString formatMprisTimestamp(qint64 positionUs, qint64 lengthUs) const;
    void refreshMprisState();
    void clearMprisState();
    void updateMprisPlaybackTicker();
    void updateMprisTimestampFromTicker();
    void connectMprisSignalObservers();
    void updateMprisPropertiesSubscription(const QString &serviceName);
    void clearMprisPropertiesSubscription();
    bool invokeMprisPlayerMethod(const QString &method);
    void persistControlCenterState() const;

    bool m_enabled = true;
    int m_currentWorkspace = 1;
    int m_workspaceCount = 1;
    QString m_mediaTitle;
    QString m_mediaArtist;
    QString m_mediaTimestamp;
    QString m_mediaArtSource;
    bool m_mediaPlaying = false;
    bool m_mprisVisible = false;
    QString m_focusedWindowTitle;
    QString m_focusedWindowIconName;
    QString m_controlCenterIconMode;
    QString m_prototypeNetworkState;
    QString m_prototypeBluetoothState;
    QString m_prototypeVolumeState;
    QStringList m_controlCenterPowerProfiles;
    QString m_controlCenterPowerProfileCurrent;
    QString m_controlCenterVolumePercentage;
    bool m_controlCenterBatteryCharging = false;
    QString m_controlCenterBatteryPercentage;
    QString m_userConfigPath;
    QLocalSocket *m_hyprlandEventSocket = nullptr;
    QByteArray m_hyprlandEventBuffer;
    QTimer *m_mprisRefreshTimer = nullptr;
    QTimer *m_mprisPlaybackTimer = nullptr;
    QString m_mprisServiceName;
    QString m_mprisPropertiesServiceName;
    qint64 m_mprisTrackLengthUs = 0;
    qint64 m_mprisPositionUs = 0;
    qint64 m_mprisLastPositionUs = 0;
    qint64 m_mprisLastPositionEpochMs = 0;
};
