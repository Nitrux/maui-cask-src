#include "valenzbridge.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QSettings>
#include <QTextStream>
#include <QtGlobal>

namespace
{
constexpr auto kDistroConfigPath = "/etc/valenz/valenz.conf";
constexpr auto kWorkspaceCurrentKey = "Workspace/currentWorkspace";
constexpr auto kWorkspaceCountKey = "Workspace/workspaceCount";
constexpr auto kDummyModeKey = "Prototype/dummyMode";
constexpr auto kDummyWorkspaceLabelKey = "Prototype/workspaceLabel";
constexpr auto kMprisTitleKey = "Mpris/title";
constexpr auto kMprisArtistKey = "Mpris/artist";
constexpr auto kMprisTimestampKey = "Mpris/timestamp";
constexpr auto kMprisArtSourceKey = "Mpris/artSource";
constexpr auto kMprisPlayingKey = "Mpris/isPlaying";
constexpr auto kMprisVisibleKey = "Mpris/visible";
constexpr auto kFocusedWindowTitleKey = "Window/focusedWindowTitle";
constexpr auto kFocusedWindowIconNameKey = "Window/focusedWindowIconName";
constexpr auto kControlCenterIconModeKey = "ControlCenter/iconMode";
constexpr auto kControlCenterPrototypeNetworkStateKey = "ControlCenter/prototypeNetworkState";
constexpr auto kControlCenterPrototypeBluetoothStateKey = "ControlCenter/prototypeBluetoothState";
constexpr auto kControlCenterPrototypeVolumeStateKey = "ControlCenter/prototypeVolumeState";
constexpr auto kControlCenterPowerProfilesKey = "ControlCenter/powerProfiles";
constexpr auto kControlCenterPowerProfileCurrentKey = "ControlCenter/powerProfileCurrent";
constexpr auto kControlCenterVolumePercentageKey = "ControlCenter/volumePercentage";
constexpr auto kControlCenterBatteryStateKey = "ControlCenter/batteryState";
constexpr auto kControlCenterBatteryPercentageKey = "ControlCenter/batteryPercentage";

constexpr auto kLegacyWorkspaceCurrentKey = "workspace/current";
constexpr auto kLegacyWorkspaceCountKey = "workspace/count";
constexpr auto kLegacyDummyModeKey = "prototype/dummyMode";
constexpr auto kLegacyDummyWorkspaceLabelKey = "prototype/workspaceLabel";
constexpr auto kLegacyMprisTitleKey = "mpris/title";
constexpr auto kLegacyMprisArtistKey = "mpris/artist";
constexpr auto kLegacyMprisTimestampKey = "mpris/timestamp";
constexpr auto kLegacyMprisArtSourceKey = "mpris/artSource";
constexpr auto kLegacyMprisPlayingKey = "mpris/playing";
constexpr auto kLegacyMprisVisibleKey = "mpris/visible";
constexpr auto kLegacyFocusedWindowTitleKey = "window/title";
constexpr auto kLegacyFocusedWindowIconNameKey = "window/iconName";
constexpr auto kLegacyControlCenterIconModeKey = "controlCenter/iconMode";
constexpr auto kLegacyControlCenterPrototypeNetworkStateKey = "controlCenter/prototypeNetworkState";
constexpr auto kLegacyControlCenterPrototypeBluetoothStateKey = "controlCenter/prototypeBluetoothState";
constexpr auto kLegacyControlCenterPrototypeVolumeStateKey = "controlCenter/prototypeVolumeState";
constexpr auto kLegacyControlCenterPowerProfilesKey = "controlCenter/powerProfiles";
constexpr auto kLegacyControlCenterPowerProfileCurrentKey = "controlCenter/powerProfileCurrent";
constexpr auto kLegacyControlCenterVolumePercentageKey = "controlCenter/volumePercentage";
constexpr auto kLegacyControlCenterBatteryStateKey = "controlCenter/batteryState";
constexpr auto kLegacyControlCenterBatteryPercentageKey = "controlCenter/batteryPercentage";

QString normalizePrototypeNetworkState(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("wired") || normalized == QLatin1String("wireless")
        || normalized == QLatin1String("hotspot") || normalized == QLatin1String("vpn")
        || normalized == QLatin1String("cellular") || normalized == QLatin1String("offline")
        || normalized == QLatin1String("auto"))
    {
        return normalized;
    }

    return QStringLiteral("auto");
}

QString normalizePrototypeBluetoothState(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("on") || normalized == QLatin1String("off")
        || normalized == QLatin1String("auto"))
    {
        return normalized;
    }

    return QStringLiteral("auto");
}

QString normalizePrototypeVolumeState(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("muted") || normalized == QLatin1String("low")
        || normalized == QLatin1String("medium") || normalized == QLatin1String("high")
        || normalized == QLatin1String("auto"))
    {
        return normalized;
    }

    return QStringLiteral("auto");
}

QString normalizeBatteryPercentage(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty())
        return QStringLiteral("0%");

    QString numeric = normalized;
    if (numeric.endsWith(QLatin1Char('%')))
        numeric.chop(1);
    numeric = numeric.trimmed();

    bool ok = false;
    const int parsed = numeric.toInt(&ok);
    if (!ok)
        return QStringLiteral("0%");

    const int bounded = qBound(0, parsed, 100);
    return QStringLiteral("%1%").arg(bounded);
}

QStringList normalizePowerProfiles(const QVariant &value)
{
    QStringList profiles;

    if (value.metaType().id() == QMetaType::QStringList)
        profiles = value.toStringList();
    else
        profiles = value.toString().split(QLatin1Char(','), Qt::SkipEmptyParts);

    QStringList normalized;
    normalized.reserve(profiles.size());

    for (const QString &profile : profiles)
    {
        const QString trimmed = profile.trimmed();
        if (trimmed.isEmpty() || normalized.contains(trimmed, Qt::CaseInsensitive))
            continue;

        normalized << trimmed;
    }

    if (normalized.isEmpty())
        normalized << QStringLiteral("power-saver") << QStringLiteral("balanced") << QStringLiteral("performance");

    return normalized;
}

QString normalizeCurrentPowerProfile(const QString &value, const QStringList &profiles)
{
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty())
        return profiles.value(0, QStringLiteral("balanced"));

    for (const QString &profile : profiles)
    {
        if (profile.compare(trimmed, Qt::CaseInsensitive) == 0)
            return profile;
    }

    return profiles.value(0, QStringLiteral("balanced"));
}

bool normalizeControlCenterBatteryCharging(const QVariant &value)
{
    if (value.metaType().id() == QMetaType::Bool)
        return value.toBool();

    const QString normalized = value.toString().trimmed().toLower();
    if (normalized == QLatin1String("charging")
        || normalized == QLatin1String("true")
        || normalized == QLatin1String("1")
        || normalized == QLatin1String("on")
        || normalized == QLatin1String("yes")
        || normalized.contains(QLatin1String("charging")))
    {
        return true;
    }

    return false;
}

QString readIniValueFallback(const QString &filePath, const QString &groupName, const QString &keyName)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QTextStream stream(&file);
    bool inTargetGroup = false;

    while (!stream.atEnd())
    {
        const QString rawLine = stream.readLine();
        const QString trimmedLine = rawLine.trimmed();

        if (trimmedLine.isEmpty() || trimmedLine.startsWith(QLatin1Char('#'))
            || trimmedLine.startsWith(QLatin1Char(';')))
        {
            continue;
        }

        if (trimmedLine.startsWith(QLatin1Char('[')) && trimmedLine.endsWith(QLatin1Char(']')))
        {
            const QString currentGroup = trimmedLine.mid(1, trimmedLine.size() - 2).trimmed();
            inTargetGroup = currentGroup.compare(groupName, Qt::CaseInsensitive) == 0;
            continue;
        }

        if (!inTargetGroup)
            continue;

        const int separatorIndex = trimmedLine.indexOf(QLatin1Char('='));
        if (separatorIndex <= 0)
            continue;

        const QString currentKey = trimmedLine.left(separatorIndex).trimmed();
        if (currentKey.compare(keyName, Qt::CaseInsensitive) != 0)
            continue;

        return trimmedLine.mid(separatorIndex + 1).trimmed();
    }

    return {};
}
}

ValenzBridge::ValenzBridge(QObject *parent)
    : QObject(parent)
{
    initializeConfig();
}

bool ValenzBridge::enabled() const
{
    return m_enabled;
}

void ValenzBridge::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    Q_EMIT enabledChanged(m_enabled);
}

int ValenzBridge::currentWorkspace() const
{
    return m_currentWorkspace;
}

void ValenzBridge::setCurrentWorkspace(int workspace)
{
    const int clampedWorkspace = clampWorkspace(workspace);
    if (m_currentWorkspace == clampedWorkspace)
        return;

    m_currentWorkspace = clampedWorkspace;
    persistWorkspaceState();
    Q_EMIT currentWorkspaceChanged(m_currentWorkspace);
}

int ValenzBridge::workspaceCount() const
{
    return m_workspaceCount;
}

void ValenzBridge::setWorkspaceCount(int count)
{
    const int normalizedCount = qMax(1, count);
    if (m_workspaceCount == normalizedCount)
        return;

    m_workspaceCount = normalizedCount;
    persistWorkspaceState();
    Q_EMIT workspaceCountChanged(m_workspaceCount);

    const int clampedCurrent = clampWorkspace(m_currentWorkspace);
    if (m_currentWorkspace != clampedCurrent)
    {
        m_currentWorkspace = clampedCurrent;
        persistWorkspaceState();
        Q_EMIT currentWorkspaceChanged(m_currentWorkspace);
    }
}

QString ValenzBridge::mediaTitle() const
{
    return m_mediaTitle;
}

void ValenzBridge::setMediaTitle(const QString &title)
{
    if (m_mediaTitle == title)
        return;

    m_mediaTitle = title;
    persistMediaState();
    Q_EMIT mediaTitleChanged(m_mediaTitle);
}

QString ValenzBridge::mediaArtist() const
{
    return m_mediaArtist;
}

void ValenzBridge::setMediaArtist(const QString &artist)
{
    if (m_mediaArtist == artist)
        return;

    m_mediaArtist = artist;
    persistMediaState();
    Q_EMIT mediaArtistChanged(m_mediaArtist);
}

QString ValenzBridge::mediaTimestamp() const
{
    return m_mediaTimestamp;
}

void ValenzBridge::setMediaTimestamp(const QString &timestamp)
{
    if (m_mediaTimestamp == timestamp)
        return;

    m_mediaTimestamp = timestamp;
    persistMediaState();
    Q_EMIT mediaTimestampChanged(m_mediaTimestamp);
}

QString ValenzBridge::mediaArtSource() const
{
    return m_mediaArtSource;
}

void ValenzBridge::setMediaArtSource(const QString &source)
{
    if (m_mediaArtSource == source)
        return;

    m_mediaArtSource = source;
    persistMediaState();
    Q_EMIT mediaArtSourceChanged(m_mediaArtSource);
}

bool ValenzBridge::mediaPlaying() const
{
    return m_mediaPlaying;
}

void ValenzBridge::setMediaPlaying(bool playing)
{
    if (m_mediaPlaying == playing)
        return;

    m_mediaPlaying = playing;
    persistMediaState();
    Q_EMIT mediaPlayingChanged(m_mediaPlaying);
}

bool ValenzBridge::mprisVisible() const
{
    return m_mprisVisible;
}

void ValenzBridge::setMprisVisible(bool visible)
{
    if (m_mprisVisible == visible)
        return;

    m_mprisVisible = visible;
    persistMediaState();
    Q_EMIT mprisVisibleChanged(m_mprisVisible);
}

QString ValenzBridge::focusedWindowTitle() const
{
    return m_focusedWindowTitle;
}

void ValenzBridge::setFocusedWindowTitle(const QString &title)
{
    if (m_focusedWindowTitle == title)
        return;

    m_focusedWindowTitle = title;
    persistFocusedWindowState();
    Q_EMIT focusedWindowTitleChanged(m_focusedWindowTitle);
}

QString ValenzBridge::focusedWindowIconName() const
{
    return m_focusedWindowIconName;
}

void ValenzBridge::setFocusedWindowIconName(const QString &iconName)
{
    if (m_focusedWindowIconName == iconName)
        return;

    m_focusedWindowIconName = iconName;
    persistFocusedWindowState();
    Q_EMIT focusedWindowIconNameChanged(m_focusedWindowIconName);
}

QString ValenzBridge::controlCenterIconMode() const
{
    return m_controlCenterIconMode;
}

void ValenzBridge::setControlCenterIconMode(const QString &mode)
{
    if (m_controlCenterIconMode == mode)
        return;

    m_controlCenterIconMode = mode;
    persistControlCenterState();
    Q_EMIT controlCenterIconModeChanged(m_controlCenterIconMode);
}

QString ValenzBridge::prototypeNetworkState() const
{
    return m_prototypeNetworkState;
}

void ValenzBridge::setPrototypeNetworkState(const QString &state)
{
    const QString normalized = normalizePrototypeNetworkState(state);
    if (m_prototypeNetworkState == normalized)
        return;

    m_prototypeNetworkState = normalized;
    persistControlCenterState();
    Q_EMIT prototypeNetworkStateChanged(m_prototypeNetworkState);
}

QString ValenzBridge::prototypeBluetoothState() const
{
    return m_prototypeBluetoothState;
}

void ValenzBridge::setPrototypeBluetoothState(const QString &state)
{
    const QString normalized = normalizePrototypeBluetoothState(state);
    if (m_prototypeBluetoothState == normalized)
        return;

    m_prototypeBluetoothState = normalized;
    persistControlCenterState();
    Q_EMIT prototypeBluetoothStateChanged(m_prototypeBluetoothState);
}

QString ValenzBridge::prototypeVolumeState() const
{
    return m_prototypeVolumeState;
}

void ValenzBridge::setPrototypeVolumeState(const QString &state)
{
    const QString normalized = normalizePrototypeVolumeState(state);
    if (m_prototypeVolumeState == normalized)
        return;

    m_prototypeVolumeState = normalized;
    persistControlCenterState();
    Q_EMIT prototypeVolumeStateChanged(m_prototypeVolumeState);
}

QStringList ValenzBridge::controlCenterPowerProfiles() const
{
    return m_controlCenterPowerProfiles;
}

void ValenzBridge::setControlCenterPowerProfiles(const QStringList &profiles)
{
    const QStringList normalizedProfiles = normalizePowerProfiles(profiles);
    const QString normalizedCurrent = normalizeCurrentPowerProfile(m_controlCenterPowerProfileCurrent, normalizedProfiles);

    const bool profilesChanged = m_controlCenterPowerProfiles != normalizedProfiles;
    const bool currentChanged = m_controlCenterPowerProfileCurrent != normalizedCurrent;
    if (!profilesChanged && !currentChanged)
        return;

    m_controlCenterPowerProfiles = normalizedProfiles;
    m_controlCenterPowerProfileCurrent = normalizedCurrent;
    persistControlCenterState();

    if (profilesChanged)
        Q_EMIT controlCenterPowerProfilesChanged(m_controlCenterPowerProfiles);
    if (currentChanged)
        Q_EMIT controlCenterPowerProfileCurrentChanged(m_controlCenterPowerProfileCurrent);
}

QString ValenzBridge::controlCenterPowerProfileCurrent() const
{
    return m_controlCenterPowerProfileCurrent;
}

void ValenzBridge::setControlCenterPowerProfileCurrent(const QString &profile)
{
    const QString normalized = normalizeCurrentPowerProfile(profile, m_controlCenterPowerProfiles);
    if (m_controlCenterPowerProfileCurrent == normalized)
        return;

    m_controlCenterPowerProfileCurrent = normalized;
    persistControlCenterState();
    Q_EMIT controlCenterPowerProfileCurrentChanged(m_controlCenterPowerProfileCurrent);
}

QString ValenzBridge::controlCenterVolumePercentage() const
{
    return m_controlCenterVolumePercentage;
}

void ValenzBridge::setControlCenterVolumePercentage(const QString &value)
{
    const QString normalized = normalizeBatteryPercentage(value);
    if (m_controlCenterVolumePercentage == normalized)
        return;

    m_controlCenterVolumePercentage = normalized;
    persistControlCenterState();
    Q_EMIT controlCenterVolumePercentageChanged(m_controlCenterVolumePercentage);
}

bool ValenzBridge::controlCenterBatteryCharging() const
{
    return m_controlCenterBatteryCharging;
}

void ValenzBridge::setControlCenterBatteryCharging(bool charging)
{
    if (m_controlCenterBatteryCharging == charging)
        return;

    m_controlCenterBatteryCharging = charging;
    persistControlCenterState();
    Q_EMIT controlCenterBatteryChargingChanged(m_controlCenterBatteryCharging);
}

QString ValenzBridge::controlCenterBatteryPercentage() const
{
    return m_controlCenterBatteryPercentage;
}

void ValenzBridge::setControlCenterBatteryPercentage(const QString &value)
{
    const QString normalized = normalizeBatteryPercentage(value);
    if (m_controlCenterBatteryPercentage == normalized)
        return;

    m_controlCenterBatteryPercentage = normalized;
    persistControlCenterState();
    Q_EMIT controlCenterBatteryPercentageChanged(m_controlCenterBatteryPercentage);
}

void ValenzBridge::trace(const QString &source, const QString &action, const QString &detail)
{
    if (!m_enabled)
        return;

    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    Q_EMIT traceRaised(source, action, detail, timestamp);
}

void ValenzBridge::goToPreviousWorkspace()
{
    if (m_currentWorkspace <= 1)
        return;

    setCurrentWorkspace(m_currentWorkspace - 1);
    trace(QStringLiteral("workspace"), QStringLiteral("previous"), QString::number(m_currentWorkspace));
}

void ValenzBridge::goToNextWorkspace()
{
    if (m_currentWorkspace >= m_workspaceCount)
        return;

    setCurrentWorkspace(m_currentWorkspace + 1);
    trace(QStringLiteral("workspace"), QStringLiteral("next"), QString::number(m_currentWorkspace));
}

void ValenzBridge::setWorkspaceState(int currentWorkspace, int workspaceCount)
{
    setWorkspaceCount(workspaceCount);
    setCurrentWorkspace(currentWorkspace);
}

void ValenzBridge::mediaPreviousTrack()
{
    trace(QStringLiteral("mpris"), QStringLiteral("previous_track"));
}

void ValenzBridge::mediaTogglePlayPause()
{
    setMediaPlaying(!m_mediaPlaying);
    trace(QStringLiteral("mpris"), m_mediaPlaying ? QStringLiteral("play") : QStringLiteral("pause"));
}

void ValenzBridge::mediaNextTrack()
{
    trace(QStringLiteral("mpris"), QStringLiteral("next_track"));
}

QString ValenzBridge::configFilePath() const
{
    return m_userConfigPath;
}

int ValenzBridge::clampWorkspace(int workspace) const
{
    return qBound(1, workspace, m_workspaceCount);
}

void ValenzBridge::initializeConfig()
{
    const QString configDir = QDir::home().filePath(".config/valenz");
    QDir dir;
    dir.mkpath(configDir);

    m_userConfigPath = configDir + "/valenz.conf";

    QSettings userSettings(m_userConfigPath, QSettings::IniFormat);

    if (QFileInfo::exists(kDistroConfigPath))
    {
        QSettings distroSettings(QString::fromLatin1(kDistroConfigPath), QSettings::IniFormat);
        const QStringList distroKeys = distroSettings.allKeys();
        for (const QString &key : distroKeys)
        {
            if (!userSettings.contains(key))
            {
                userSettings.setValue(key, distroSettings.value(key));
            }
        }
    }

    const auto ensureKey = [&userSettings](const QString &newKey, const QString &legacyKey, const QVariant &defaultValue)
    {
        if (userSettings.contains(newKey))
            return;

        if (userSettings.contains(legacyKey))
        {
            userSettings.setValue(newKey, userSettings.value(legacyKey));
            userSettings.remove(legacyKey);
            return;
        }

        userSettings.setValue(newKey, defaultValue);
    };

    ensureKey(QString::fromLatin1(kWorkspaceCurrentKey), QString::fromLatin1(kLegacyWorkspaceCurrentKey), 1);
    ensureKey(QString::fromLatin1(kWorkspaceCountKey), QString::fromLatin1(kLegacyWorkspaceCountKey), 10);
    ensureKey(QString::fromLatin1(kDummyModeKey), QString::fromLatin1(kLegacyDummyModeKey), true);
    ensureKey(QString::fromLatin1(kDummyWorkspaceLabelKey), QString::fromLatin1(kLegacyDummyWorkspaceLabelKey), "Hyprland Workspace");
    ensureKey(QString::fromLatin1(kMprisTitleKey), QString::fromLatin1(kLegacyMprisTitleKey), "Prototype Track");
    ensureKey(QString::fromLatin1(kMprisArtistKey), QString::fromLatin1(kLegacyMprisArtistKey), "Prototype Artist");
    ensureKey(QString::fromLatin1(kMprisTimestampKey), QString::fromLatin1(kLegacyMprisTimestampKey), "00:42 / 03:17");
    ensureKey(QString::fromLatin1(kMprisArtSourceKey), QString::fromLatin1(kLegacyMprisArtSourceKey), "");
    ensureKey(QString::fromLatin1(kMprisPlayingKey), QString::fromLatin1(kLegacyMprisPlayingKey), false);
    ensureKey(QString::fromLatin1(kMprisVisibleKey), QString::fromLatin1(kLegacyMprisVisibleKey), true);
    ensureKey(QString::fromLatin1(kFocusedWindowTitleKey), QString::fromLatin1(kLegacyFocusedWindowTitleKey), "Focused window title");
    ensureKey(QString::fromLatin1(kFocusedWindowIconNameKey), QString::fromLatin1(kLegacyFocusedWindowIconNameKey), "application-x-executable");
    ensureKey(QString::fromLatin1(kControlCenterIconModeKey), QString::fromLatin1(kLegacyControlCenterIconModeKey), "auto");
    ensureKey(QString::fromLatin1(kControlCenterPrototypeNetworkStateKey), QString::fromLatin1(kLegacyControlCenterPrototypeNetworkStateKey), "auto");
    ensureKey(QString::fromLatin1(kControlCenterPrototypeBluetoothStateKey), QString::fromLatin1(kLegacyControlCenterPrototypeBluetoothStateKey), "auto");
    ensureKey(QString::fromLatin1(kControlCenterPrototypeVolumeStateKey), QString::fromLatin1(kLegacyControlCenterPrototypeVolumeStateKey), "auto");
    ensureKey(QString::fromLatin1(kControlCenterPowerProfilesKey), QString::fromLatin1(kLegacyControlCenterPowerProfilesKey),
              QStringList { QStringLiteral("power-saver"), QStringLiteral("balanced"), QStringLiteral("performance") });
    ensureKey(QString::fromLatin1(kControlCenterPowerProfileCurrentKey), QString::fromLatin1(kLegacyControlCenterPowerProfileCurrentKey), "balanced");
    ensureKey(QString::fromLatin1(kControlCenterVolumePercentageKey), QString::fromLatin1(kLegacyControlCenterVolumePercentageKey), "50%");

    if (!userSettings.contains(kControlCenterBatteryStateKey))
    {
        if (userSettings.contains(kLegacyControlCenterBatteryStateKey))
        {
            userSettings.setValue(kControlCenterBatteryStateKey, userSettings.value(kLegacyControlCenterBatteryStateKey));
            userSettings.remove(kLegacyControlCenterBatteryStateKey);
        }
        else
        {
            const QVariant legacyBatteryIcon = userSettings.contains("ControlCenter/batteryIconName")
                                                   ? userSettings.value("ControlCenter/batteryIconName")
                                                   : userSettings.value("controlCenter/batteryIconName");
            userSettings.setValue(kControlCenterBatteryStateKey,
                                  normalizeControlCenterBatteryCharging(legacyBatteryIcon) ? "charging" : "battery");
        }
    }

    ensureKey(QString::fromLatin1(kControlCenterBatteryPercentageKey), QString::fromLatin1(kLegacyControlCenterBatteryPercentageKey), "0%");

    userSettings.remove("ControlCenter/batteryIconName");
    userSettings.remove("controlCenter/batteryIconName");
    userSettings.remove("ControlCenter/powerProfileIconName");
    userSettings.remove("controlCenter/powerProfileIconName");

    userSettings.sync();

    const QString focusedTitleKey = QString::fromLatin1(kFocusedWindowTitleKey);
    const QStringList allKeys = userSettings.allKeys();
    qInfo().noquote() << QStringLiteral("[valenz][config] qsettings.status=%1 contains(%2)=%3")
                             .arg(static_cast<int>(userSettings.status()))
                             .arg(focusedTitleKey, userSettings.contains(focusedTitleKey) ? QStringLiteral("true") : QStringLiteral("false"));
    qInfo().noquote() << QStringLiteral("[valenz][config] allKeys=%1")
                             .arg(allKeys.join(QStringLiteral(",")));

    m_workspaceCount = qMax(1, userSettings.value(kWorkspaceCountKey, 10).toInt());
    m_currentWorkspace = qBound(1, userSettings.value(kWorkspaceCurrentKey, 1).toInt(), m_workspaceCount);
    m_mediaTitle = userSettings.value(kMprisTitleKey, "Prototype Track").toString();
    m_mediaArtist = userSettings.value(kMprisArtistKey, "Prototype Artist").toString();
    m_mediaTimestamp = userSettings.value(kMprisTimestampKey, "00:42 / 03:17").toString();
    m_mediaArtSource = userSettings.value(kMprisArtSourceKey, "").toString();
    m_mediaPlaying = userSettings.value(kMprisPlayingKey, false).toBool();
    m_mprisVisible = userSettings.value(kMprisVisibleKey, true).toBool();
    m_focusedWindowTitle = userSettings.value(kFocusedWindowTitleKey, "Focused window title").toString();
    m_focusedWindowIconName = userSettings.value(kFocusedWindowIconNameKey, "application-x-executable").toString();
    m_controlCenterIconMode = userSettings.value(kControlCenterIconModeKey, "auto").toString();
    m_prototypeNetworkState = normalizePrototypeNetworkState(userSettings.value(kControlCenterPrototypeNetworkStateKey, "auto").toString());
    m_prototypeBluetoothState = normalizePrototypeBluetoothState(userSettings.value(kControlCenterPrototypeBluetoothStateKey, "auto").toString());
    m_prototypeVolumeState = normalizePrototypeVolumeState(userSettings.value(kControlCenterPrototypeVolumeStateKey, "auto").toString());
    m_controlCenterPowerProfiles = normalizePowerProfiles(userSettings.value(kControlCenterPowerProfilesKey,
                                                                             QStringList { QStringLiteral("power-saver"),
                                                                                           QStringLiteral("balanced"),
                                                                                           QStringLiteral("performance") }));
    m_controlCenterPowerProfileCurrent = normalizeCurrentPowerProfile(userSettings.value(kControlCenterPowerProfileCurrentKey, "balanced").toString(),
                                                                      m_controlCenterPowerProfiles);
    m_controlCenterVolumePercentage = normalizeBatteryPercentage(userSettings.value(kControlCenterVolumePercentageKey, "50%").toString());
    m_controlCenterBatteryCharging = normalizeControlCenterBatteryCharging(userSettings.value(kControlCenterBatteryStateKey, "battery"));
    m_controlCenterBatteryPercentage = normalizeBatteryPercentage(userSettings.value(kControlCenterBatteryPercentageKey, "0%").toString());

    if (m_focusedWindowTitle.isEmpty())
    {
        const QString fallbackTitle = readIniValueFallback(m_userConfigPath, QStringLiteral("Window"),
                                                           QStringLiteral("focusedWindowTitle"));
        if (!fallbackTitle.isEmpty())
        {
            m_focusedWindowTitle = fallbackTitle;
            qWarning().noquote() << QStringLiteral("[valenz][config] recovered focusedWindowTitle via fallback parser");
        }
    }

    qInfo().noquote() << QStringLiteral("[valenz][config] path=%1 exists=%2")
                             .arg(m_userConfigPath, QFileInfo::exists(m_userConfigPath) ? QStringLiteral("true") : QStringLiteral("false"));
    qInfo().noquote() << QStringLiteral("[valenz][config] loaded Window/focusedWindowTitle=%1")
                             .arg(m_focusedWindowTitle);
}

void ValenzBridge::persistWorkspaceState() const
{
    if (m_userConfigPath.isEmpty())
        return;

    QSettings userSettings(m_userConfigPath, QSettings::IniFormat);
    userSettings.setValue(kWorkspaceCurrentKey, m_currentWorkspace);
    userSettings.setValue(kWorkspaceCountKey, m_workspaceCount);
    userSettings.sync();
}

void ValenzBridge::persistMediaState() const
{
    if (m_userConfigPath.isEmpty())
        return;

    QSettings userSettings(m_userConfigPath, QSettings::IniFormat);
    userSettings.setValue(kMprisTitleKey, m_mediaTitle);
    userSettings.setValue(kMprisArtistKey, m_mediaArtist);
    userSettings.setValue(kMprisTimestampKey, m_mediaTimestamp);
    userSettings.setValue(kMprisArtSourceKey, m_mediaArtSource);
    userSettings.setValue(kMprisPlayingKey, m_mediaPlaying);
    userSettings.setValue(kMprisVisibleKey, m_mprisVisible);
    userSettings.sync();
}

void ValenzBridge::persistFocusedWindowState() const
{
    if (m_userConfigPath.isEmpty())
        return;

    QSettings userSettings(m_userConfigPath, QSettings::IniFormat);
    userSettings.setValue(kFocusedWindowTitleKey, m_focusedWindowTitle);
    userSettings.setValue(kFocusedWindowIconNameKey, m_focusedWindowIconName);
    userSettings.sync();
}

void ValenzBridge::persistControlCenterState() const
{
    if (m_userConfigPath.isEmpty())
        return;

    QSettings userSettings(m_userConfigPath, QSettings::IniFormat);
    userSettings.setValue(kControlCenterIconModeKey, m_controlCenterIconMode);
    userSettings.setValue(kControlCenterPrototypeNetworkStateKey, m_prototypeNetworkState);
    userSettings.setValue(kControlCenterPrototypeBluetoothStateKey, m_prototypeBluetoothState);
    userSettings.setValue(kControlCenterPrototypeVolumeStateKey, m_prototypeVolumeState);
    userSettings.setValue(kControlCenterPowerProfilesKey, m_controlCenterPowerProfiles);
    userSettings.setValue(kControlCenterPowerProfileCurrentKey, m_controlCenterPowerProfileCurrent);
    userSettings.setValue(kControlCenterVolumePercentageKey, m_controlCenterVolumePercentage);
    userSettings.setValue(kControlCenterBatteryStateKey, m_controlCenterBatteryCharging ? "charging" : "battery");
    userSettings.setValue(kControlCenterBatteryPercentageKey, m_controlCenterBatteryPercentage);
    userSettings.sync();
}
