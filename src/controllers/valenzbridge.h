#pragma once

#include <QObject>
#include <QString>

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

    Q_INVOKABLE void trace(const QString &source, const QString &action, const QString &detail = QString());
    Q_INVOKABLE void goToPreviousWorkspace();
    Q_INVOKABLE void goToNextWorkspace();
    Q_INVOKABLE void setWorkspaceState(int currentWorkspace, int workspaceCount);
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

private:
    int clampWorkspace(int workspace) const;
    void initializeConfig();
    void persistWorkspaceState() const;
    void persistMediaState() const;
    void persistFocusedWindowState() const;
    void persistControlCenterState() const;

    bool m_enabled = true;
    int m_currentWorkspace = 1;
    int m_workspaceCount = 1;
    QString m_mediaTitle;
    QString m_mediaArtist;
    QString m_mediaTimestamp;
    QString m_mediaArtSource;
    bool m_mediaPlaying = false;
    bool m_mprisVisible = true;
    QString m_focusedWindowTitle;
    QString m_focusedWindowIconName;
    QString m_controlCenterIconMode;
    QString m_prototypeNetworkState;
    QString m_prototypeBluetoothState;
    QString m_prototypeVolumeState;
    QString m_userConfigPath;
};
