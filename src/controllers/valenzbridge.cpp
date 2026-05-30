#include "valenzbridge.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaType>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusVariant>
#include <QLocalSocket>
#include <QTimer>
#include <QProcess>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>
#include <QtGlobal>
#include <utility>

namespace
{
constexpr auto kDistroConfigPath = "/etc/valenz/valenz.conf";
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

constexpr auto kMprisServicePrefix = "org.mpris.MediaPlayer2.";
constexpr auto kMprisObjectPath = "/org/mpris/MediaPlayer2";
constexpr auto kMprisPlayerInterface = "org.mpris.MediaPlayer2.Player";
constexpr auto kDbusPropertiesInterface = "org.freedesktop.DBus.Properties";
constexpr auto kDbusService = "org.freedesktop.DBus";
constexpr auto kDbusPath = "/org/freedesktop/DBus";
constexpr auto kDbusInterface = "org.freedesktop.DBus";
constexpr auto kMprisRefreshIntervalMs = 4000;
constexpr auto kMprisPlaybackTickMs = 250;

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

QVariant unwrapMprisVariant(const QVariant &value)
{
    if (value.metaType() == QMetaType::fromType<QDBusVariant>())
        return value.value<QDBusVariant>().variant();

    return value;
}

QVariantMap variantToVariantMap(const QVariant &value)
{
    const QVariant unwrapped = unwrapMprisVariant(value);

    if (unwrapped.metaType().id() == QMetaType::QVariantMap || unwrapped.canConvert<QVariantMap>())
        return unwrapped.toMap();

    if (unwrapped.metaType() == QMetaType::fromType<QDBusArgument>())
    {
        const QDBusArgument argument = unwrapped.value<QDBusArgument>();
        const QVariantMap decodedMap = qdbus_cast<QVariantMap>(argument);
        if (!decodedMap.isEmpty())
            return decodedMap;
    }

    return {};
}

QStringList variantToStringList(const QVariant &value)
{
    const QVariant unwrapped = unwrapMprisVariant(value);

    if (unwrapped.metaType().id() == QMetaType::QStringList)
        return unwrapped.toStringList();

    if (unwrapped.canConvert<QStringList>())
        return unwrapped.value<QStringList>();

    if (unwrapped.metaType() == QMetaType::fromType<QVariantList>())
    {
        QStringList strings;
        const QVariantList values = unwrapped.toList();

        for (const QVariant &entry : values)
        {
            const QString item = unwrapMprisVariant(entry).toString().trimmed();
            if (!item.isEmpty())
                strings << item;
        }

        return strings;
    }

    if (unwrapped.metaType() == QMetaType::fromType<QDBusArgument>())
    {
        const QDBusArgument argument = unwrapped.value<QDBusArgument>();
        const QStringList decodedList = qdbus_cast<QStringList>(argument);
        if (!decodedList.isEmpty())
            return decodedList;
    }

    const QString singleValue = unwrapped.toString().trimmed();
    return singleValue.isEmpty() ? QStringList{} : QStringList{singleValue};
}

bool runHyprctlJson(const QStringList &arguments, QJsonValue *result)
{
    QProcess process;
    process.start(QStringLiteral("hyprctl"), arguments);

    if (!process.waitForStarted(250) || !process.waitForFinished(1200))
        return false;

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return false;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(process.readAllStandardOutput(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return false;

    if (document.isObject())
        *result = document.object();
    else if (document.isArray())
        *result = document.array();
    else
        return false;

    return true;
}

bool runHyprctlDispatch(const QStringList &arguments)
{
    QProcess process;
    process.start(QStringLiteral("hyprctl"), arguments);

    if (!process.waitForStarted(250))
    {
        return false;
    }

    if (!process.waitForFinished(1200))
    {
        process.kill();
        process.waitForFinished(250);
        return false;
    }

    const QString stdOut = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    const QString stdErr = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    const bool processOk = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    const bool outputHasError = stdOut.contains(QStringLiteral("error:"), Qt::CaseInsensitive)
                             || stdErr.contains(QStringLiteral("error:"), Qt::CaseInsensitive);
    const bool outputLooksOk = stdOut.isEmpty() || stdOut.startsWith(QStringLiteral("ok"), Qt::CaseInsensitive);
    const bool ok = processOk && !outputHasError && outputLooksOk;

    return ok;
}

bool dispatchWorkspaceFocus(const QString &selector)
{
    const QString luaDispatch = QStringLiteral("hl.dsp.focus({ workspace = \"%1\" })").arg(selector);

    if (runHyprctlDispatch(QStringList { QStringLiteral("dispatch"), luaDispatch }))
        return true;

    return runHyprctlDispatch(QStringList { QStringLiteral("dispatch"), QStringLiteral("workspace"), selector });
}

QString hyprlandEventSocketPath()
{
    const QString signature = QString::fromLocal8Bit(qgetenv("HYPRLAND_INSTANCE_SIGNATURE")).trimmed();
    if (signature.isEmpty())
        return {};

    QString runtimeDir = QString::fromLocal8Bit(qgetenv("XDG_RUNTIME_DIR")).trimmed();
    if (runtimeDir.isEmpty())
        runtimeDir = QStringLiteral("/tmp");

    return QStringLiteral("%1/hypr/%2/.socket2.sock").arg(runtimeDir, signature);
}

bool isWorkspaceRelatedHyprlandEvent(const QString &eventName)
{
    return eventName == QLatin1String("workspace")
        || eventName == QLatin1String("workspacev2")
        || eventName == QLatin1String("focusedmon")
        || eventName == QLatin1String("focusedmonv2")
        || eventName == QLatin1String("createworkspace")
        || eventName == QLatin1String("createworkspacev2")
        || eventName == QLatin1String("destroyworkspace")
        || eventName == QLatin1String("destroyworkspacev2")
        || eventName == QLatin1String("moveworkspace")
        || eventName == QLatin1String("moveworkspacev2");
}

bool isFocusedWindowRelatedHyprlandEvent(const QString &eventName)
{
    return eventName == QLatin1String("activewindow")
        || eventName == QLatin1String("activewindowv2")
        || eventName == QLatin1String("windowtitle")
        || eventName == QLatin1String("windowtitlev2")
        || eventName == QLatin1String("openwindow")
        || eventName == QLatin1String("closewindow");
}

int hyprlandCurrentWorkspace(const QJsonValue &activeWorkspace)
{
    if (!activeWorkspace.isObject())
        return -1;

    return activeWorkspace.toObject().value(QStringLiteral("id")).toInt(-1);
}

int hyprlandWorkspaceCount(const QJsonValue &workspaces)
{
    if (!workspaces.isArray())
        return -1;

    int maxWorkspaceId = 0;
    const QJsonArray workspaceArray = workspaces.toArray();

    for (const QJsonValue &workspaceValue : workspaceArray)
    {
        const int id = workspaceValue.toObject().value(QStringLiteral("id")).toInt(0);
        if (id > maxWorkspaceId)
            maxWorkspaceId = id;
    }

    return maxWorkspaceId > 0 ? maxWorkspaceId : workspaceArray.size();
}

void addUniqueCaseInsensitive(QStringList *list, const QString &candidate)
{
    if (!list)
        return;

    const QString trimmed = candidate.trimmed();
    if (trimmed.isEmpty())
        return;

    for (const QString &entry : std::as_const(*list))
    {
        if (entry.compare(trimmed, Qt::CaseInsensitive) == 0)
            return;
    }

    list->append(trimmed);
}

QString withoutDesktopSuffix(const QString &value)
{
    QString normalized = value.trimmed();
    if (normalized.endsWith(QLatin1String(".desktop"), Qt::CaseInsensitive))
        normalized.chop(8);

    return normalized;
}

QString normalizedLookupKey(const QString &value)
{
    QString normalized = withoutDesktopSuffix(value).toLower().simplified();
    normalized.remove(QChar::fromLatin1(32));
    normalized.remove(QChar::fromLatin1(45));
    normalized.remove(QChar::fromLatin1(95));
    normalized.remove(QChar::fromLatin1(46));
    return normalized;
}

void addLookupVariants(QStringList *variants, const QString &value)
{
    const QString base = withoutDesktopSuffix(value);
    if (base.isEmpty())
        return;

    addUniqueCaseInsensitive(variants, base);

    const QString lower = base.toLower();
    addUniqueCaseInsensitive(variants, lower);

    const QString simplified = lower.simplified();
    addUniqueCaseInsensitive(variants, simplified);

    QString compact = simplified;
    compact.remove(QChar::fromLatin1(32));
    addUniqueCaseInsensitive(variants, compact);

    const QString tail = compact.section(QChar::fromLatin1(46), -1);
    addUniqueCaseInsensitive(variants, tail);

    const QString fileName = QFileInfo(base).fileName();
    addUniqueCaseInsensitive(variants, fileName);
}

void addWindowIconCandidates(QStringList *candidates, const QString &value)
{
    const QString base = withoutDesktopSuffix(value);
    if (base.isEmpty())
        return;

    addUniqueCaseInsensitive(candidates, base);

    const QString lower = base.toLower();
    addUniqueCaseInsensitive(candidates, lower);

    QString normalized = lower;
    normalized.replace(QChar::fromLatin1(32), QChar::fromLatin1(45));
    addUniqueCaseInsensitive(candidates, normalized);

    QString compact = lower;
    compact.remove(QChar::fromLatin1(32));
    addUniqueCaseInsensitive(candidates, compact);

    QString dotted = compact;
    dotted.replace(QChar::fromLatin1(46), QChar::fromLatin1(45));
    addUniqueCaseInsensitive(candidates, dotted);

    QString underscored = dotted;
    underscored.replace(QChar::fromLatin1(95), QChar::fromLatin1(45));
    addUniqueCaseInsensitive(candidates, underscored);

    const QString dottedTail = compact.section(QChar::fromLatin1(46), -1);
    addUniqueCaseInsensitive(candidates, dottedTail);

    const QFileInfo fileInfo(base);
    addUniqueCaseInsensitive(candidates, fileInfo.fileName());
    addUniqueCaseInsensitive(candidates, fileInfo.completeBaseName());
}

QString shortCommandFromExec(const QString &execField)
{
    const QString trimmed = execField.trimmed();
    if (trimmed.isEmpty())
        return {};

    const QStringList tokens = trimmed.split(QChar::fromLatin1(32), Qt::SkipEmptyParts);
    if (tokens.isEmpty())
        return {};

    int commandIndex = 0;
    if (tokens.at(0) == QLatin1String("env"))
    {
        commandIndex = 1;
        while (commandIndex < tokens.size())
        {
            const QString token = tokens.at(commandIndex);
            if (token.startsWith(QLatin1Char('-')) || token.contains(QLatin1Char('=')))
            {
                ++commandIndex;
                continue;
            }

            break;
        }
    }

    if (commandIndex >= tokens.size())
        return {};

    QString command = tokens.at(commandIndex).trimmed();
    if (command.startsWith(QLatin1Char('"')) && command.endsWith(QLatin1Char('"')) && command.size() > 1)
        command = command.mid(1, command.size() - 2);
    else if (command.startsWith(QLatin1Char('\'')) && command.endsWith(QLatin1Char('\'')) && command.size() > 1)
        command = command.mid(1, command.size() - 2);

    return QFileInfo(command).completeBaseName();
}

void registerDesktopLookupValue(QHash<QString, QString> *iconLookup, const QString &value, const QString &iconName)
{
    if (!iconLookup)
        return;

    const QString icon = iconName.trimmed();
    if (icon.isEmpty())
        return;

    QStringList variants;
    addLookupVariants(&variants, value);

    for (const QString &variant : std::as_const(variants))
    {
        const QString key = normalizedLookupKey(variant);
        if (key.isEmpty() || iconLookup->contains(key))
            continue;

        iconLookup->insert(key, icon);
    }
}

QStringList desktopEntryDirs()
{
    QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if (dirs.isEmpty())
    {
        dirs << QDir::homePath() + QStringLiteral("/.local/share/applications")
             << QStringLiteral("/usr/local/share/applications")
             << QStringLiteral("/usr/share/applications");
    }

    QSet<QString> seen;
    QStringList uniqueDirs;
    uniqueDirs.reserve(dirs.size());

    for (const QString &dir : std::as_const(dirs))
    {
        const QString normalizedDir = QDir::cleanPath(dir.trimmed());
        if (normalizedDir.isEmpty() || seen.contains(normalizedDir))
            continue;

        seen.insert(normalizedDir);
        uniqueDirs << normalizedDir;
    }

    return uniqueDirs;
}

QHash<QString, QString> buildDesktopIconLookup()
{
    QHash<QString, QString> iconLookup;

    const QStringList dirs = desktopEntryDirs();
    for (const QString &dirPath : dirs)
    {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;

        QDirIterator iterator(dirPath,
                              QStringList { QStringLiteral("*.desktop") },
                              QDir::Files,
                              QDirIterator::Subdirectories);

        while (iterator.hasNext())
        {
            const QString filePath = iterator.next();
            QSettings desktopFile(filePath, QSettings::IniFormat);

            const QString type = desktopFile.value(QStringLiteral("Desktop Entry/Type")).toString().trimmed();
            if (!type.isEmpty() && type.compare(QStringLiteral("Application"), Qt::CaseInsensitive) != 0)
                continue;

            const QString iconName = desktopFile.value(QStringLiteral("Desktop Entry/Icon")).toString().trimmed();
            if (iconName.isEmpty())
                continue;

            const QString appId = QFileInfo(filePath).completeBaseName();
            registerDesktopLookupValue(&iconLookup, appId, iconName);

            const QString startupWmClass = desktopFile.value(QStringLiteral("Desktop Entry/StartupWMClass")).toString().trimmed();
            registerDesktopLookupValue(&iconLookup, startupWmClass, iconName);

            const QString appName = desktopFile.value(QStringLiteral("Desktop Entry/Name")).toString().trimmed();
            registerDesktopLookupValue(&iconLookup, appName, iconName);

            const QString execField = desktopFile.value(QStringLiteral("Desktop Entry/Exec")).toString().trimmed();
            registerDesktopLookupValue(&iconLookup, shortCommandFromExec(execField), iconName);
        }
    }

    return iconLookup;
}

const QHash<QString, QString> &desktopIconLookup()
{
    static const QHash<QString, QString> lookup = buildDesktopIconLookup();
    return lookup;
}

QString lookupIconFromDesktopEntries(const QString &value)
{
    const auto &lookup = desktopIconLookup();
    if (lookup.isEmpty())
        return {};

    QStringList variants;
    addLookupVariants(&variants, value);

    for (const QString &variant : std::as_const(variants))
    {
        const QString key = normalizedLookupKey(variant);
        if (key.isEmpty())
            continue;

        const auto match = lookup.constFind(key);
        if (match != lookup.constEnd() && !match.value().trimmed().isEmpty())
            return match.value().trimmed();
    }

    return {};
}

QString processNameFromPid(qint64 pid)
{
    if (pid <= 0)
        return {};

    QFile commFile(QStringLiteral("/proc/%1/comm").arg(pid));
    if (commFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        const QString comm = QString::fromLocal8Bit(commFile.readLine()).trimmed();
        if (!comm.isEmpty())
            return comm;
    }

    QFileInfo exeLink(QStringLiteral("/proc/%1/exe").arg(pid));
    if (!exeLink.exists())
        return {};

    const QString target = exeLink.symLinkTarget().trimmed();
    if (target.isEmpty())
        return {};

    return QFileInfo(target).completeBaseName();
}

bool isUsableIconSource(const QString &value)
{
    const QString candidate = value.trimmed();
    if (candidate.isEmpty())
        return false;

    return QIcon::hasThemeIcon(candidate) || QFileInfo::exists(candidate);
}


}

ValenzBridge::ValenzBridge(QObject *parent)
    : QObject(parent)
{
    initializeConfig();
    refreshWorkspaceState();
    refreshFocusedWindowState();
    connectHyprlandEventSocket();
    connectMprisSignalObservers();

    m_mprisRefreshTimer = new QTimer(this);
    m_mprisRefreshTimer->setInterval(kMprisRefreshIntervalMs);
    m_mprisRefreshTimer->setTimerType(Qt::CoarseTimer);
    connect(m_mprisRefreshTimer, &QTimer::timeout, this, &ValenzBridge::refreshMprisState);
    m_mprisRefreshTimer->start();

    m_mprisPlaybackTimer = new QTimer(this);
    m_mprisPlaybackTimer->setInterval(kMprisPlaybackTickMs);
    m_mprisPlaybackTimer->setTimerType(Qt::CoarseTimer);
    connect(m_mprisPlaybackTimer, &QTimer::timeout, this, &ValenzBridge::updateMprisTimestampFromTicker);

    refreshMprisState();
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
    Q_EMIT workspaceCountChanged(m_workspaceCount);

    const int clampedCurrent = clampWorkspace(m_currentWorkspace);
    if (m_currentWorkspace != clampedCurrent)
    {
        m_currentWorkspace = clampedCurrent;
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

    if (!dispatchWorkspaceFocus(QStringLiteral("-1")))
    {
        return;
    }
    refreshWorkspaceState();

}

void ValenzBridge::goToNextWorkspace()
{

    if (!dispatchWorkspaceFocus(QStringLiteral("+1")))
    {
        return;
    }
    refreshWorkspaceState();

}

bool ValenzBridge::refreshWorkspaceState()
{
    QJsonValue activeWorkspace;
    QJsonValue workspaces;

    if (!runHyprctlJson(QStringList { QStringLiteral("-j"), QStringLiteral("activeworkspace") }, &activeWorkspace))
    {
        return false;
    }

    if (!runHyprctlJson(QStringList { QStringLiteral("-j"), QStringLiteral("workspaces") }, &workspaces))
    {
        return false;
    }

    const int currentWorkspace = hyprlandCurrentWorkspace(activeWorkspace);
    const int workspaceCount = hyprlandWorkspaceCount(workspaces);

    if (currentWorkspace < 1 || workspaceCount < 1)
    {
        return false;
    }

    setWorkspaceCount(workspaceCount);
    setCurrentWorkspace(currentWorkspace);
    return true;
}

bool ValenzBridge::refreshFocusedWindowState()
{
    QJsonValue activeWindow;

    if (!runHyprctlJson(QStringList { QStringLiteral("-j"), QStringLiteral("activewindow") }, &activeWindow))
    {
        setFocusedWindowTitle(QString());
        setFocusedWindowIconName(QStringLiteral("application-x-executable"));
        return false;
    }

    if (!activeWindow.isObject())
    {
        setFocusedWindowTitle(QString());
        setFocusedWindowIconName(QStringLiteral("application-x-executable"));
        return false;
    }

    const QJsonObject windowObject = activeWindow.toObject();

    QString title = windowObject.value(QStringLiteral("title")).toString().trimmed();
    if (title.isEmpty())
        title = windowObject.value(QStringLiteral("initialTitle")).toString().trimmed();

    QString resolvedIconName = QStringLiteral("application-x-executable");

    QStringList iconCandidates;
    addWindowIconCandidates(&iconCandidates, windowObject.value(QStringLiteral("class")).toString());
    addWindowIconCandidates(&iconCandidates, windowObject.value(QStringLiteral("initialClass")).toString());

    const qint64 pid = windowObject.value(QStringLiteral("pid")).toVariant().toLongLong();
    addWindowIconCandidates(&iconCandidates, processNameFromPid(pid));

    for (const QString &candidate : std::as_const(iconCandidates))
    {
        if (!isUsableIconSource(candidate))
            continue;

        resolvedIconName = candidate;
        break;
    }

    if (resolvedIconName == QLatin1String("application-x-executable"))
    {
        for (const QString &candidate : std::as_const(iconCandidates))
        {
            const QString mappedIcon = lookupIconFromDesktopEntries(candidate);
            if (mappedIcon.isEmpty())
                continue;

            if (isUsableIconSource(mappedIcon))
            {
                resolvedIconName = mappedIcon;
                break;
            }

            if (resolvedIconName == QLatin1String("application-x-executable"))
                resolvedIconName = mappedIcon;
        }
    }

    setFocusedWindowTitle(title);
    setFocusedWindowIconName(resolvedIconName);
    return true;
}

void ValenzBridge::connectHyprlandEventSocket()
{
    const QString socketPath = hyprlandEventSocketPath();
    if (socketPath.isEmpty())
        return;

    if (!m_hyprlandEventSocket)
    {
        m_hyprlandEventSocket = new QLocalSocket(this);

        connect(m_hyprlandEventSocket, &QLocalSocket::readyRead, this, &ValenzBridge::handleHyprlandEventData);

        connect(m_hyprlandEventSocket, &QLocalSocket::disconnected, this,
                [this]()
        {
            m_hyprlandEventBuffer.clear();
            scheduleHyprlandEventSocketReconnect();
        });

        connect(m_hyprlandEventSocket, &QLocalSocket::errorOccurred, this,
                [this](QLocalSocket::LocalSocketError)
        {
            scheduleHyprlandEventSocketReconnect();
        });
    }

    if (m_hyprlandEventSocket->state() == QLocalSocket::ConnectedState
        || m_hyprlandEventSocket->state() == QLocalSocket::ConnectingState)
    {
        return;
    }

    m_hyprlandEventBuffer.clear();
    m_hyprlandEventSocket->abort();
    m_hyprlandEventSocket->connectToServer(socketPath, QIODevice::ReadOnly);
}

void ValenzBridge::scheduleHyprlandEventSocketReconnect()
{
    QTimer::singleShot(2000, this,
                       [this]()
    {
        connectHyprlandEventSocket();
    });
}

void ValenzBridge::handleHyprlandEventData()
{
    if (!m_hyprlandEventSocket)
        return;

    m_hyprlandEventBuffer += m_hyprlandEventSocket->readAll();

    int newlineIndex = m_hyprlandEventBuffer.indexOf('\n');
    while (newlineIndex >= 0)
    {
        const QByteArray lineBytes = m_hyprlandEventBuffer.left(newlineIndex).trimmed();
        m_hyprlandEventBuffer.remove(0, newlineIndex + 1);

        if (!lineBytes.isEmpty())
            handleHyprlandEventLine(QString::fromUtf8(lineBytes));

        newlineIndex = m_hyprlandEventBuffer.indexOf('\n');
    }
}

void ValenzBridge::handleHyprlandEventLine(const QString &line)
{
    const int separatorIndex = line.indexOf(QStringLiteral(">>"));
    if (separatorIndex <= 0)
        return;

    const QString eventName = line.left(separatorIndex).trimmed();

    if (isWorkspaceRelatedHyprlandEvent(eventName))
        refreshWorkspaceState();

    if (isFocusedWindowRelatedHyprlandEvent(eventName))
        refreshFocusedWindowState();
}

void ValenzBridge::connectMprisSignalObservers()
{
    QDBusConnection::sessionBus().connect(QString::fromLatin1(kDbusService),
                                          QString::fromLatin1(kDbusPath),
                                          QString::fromLatin1(kDbusInterface),
                                          QStringLiteral("NameOwnerChanged"),
                                          this,
                                          SLOT(onMprisNameOwnerChanged(QString,QString,QString)));
}

void ValenzBridge::updateMprisPropertiesSubscription(const QString &serviceName)
{
    if (m_mprisPropertiesServiceName == serviceName)
        return;

    clearMprisPropertiesSubscription();

    if (serviceName.isEmpty())
        return;

    const bool connected = QDBusConnection::sessionBus().connect(serviceName,
                                                                 QString::fromLatin1(kMprisObjectPath),
                                                                 QString::fromLatin1(kDbusPropertiesInterface),
                                                                 QStringLiteral("PropertiesChanged"),
                                                                 this,
                                                                 SLOT(onMprisPropertiesChanged(QString,QVariantMap,QStringList)));
    if (!connected)
    {
        return;
    }

    m_mprisPropertiesServiceName = serviceName;
}

void ValenzBridge::clearMprisPropertiesSubscription()
{
    if (m_mprisPropertiesServiceName.isEmpty())
        return;
    QDBusConnection::sessionBus().disconnect(m_mprisPropertiesServiceName,
                                             QString::fromLatin1(kMprisObjectPath),
                                             QString::fromLatin1(kDbusPropertiesInterface),
                                             QStringLiteral("PropertiesChanged"),
                                             this,
                                             SLOT(onMprisPropertiesChanged(QString,QVariantMap,QStringList)));

    m_mprisPropertiesServiceName.clear();
}

void ValenzBridge::clearMprisState()
{
    m_mprisServiceName.clear();
    m_mprisTrackLengthUs = 0;
    m_mprisPositionUs = 0;
    m_mprisLastPositionUs = 0;
    m_mprisLastPositionEpochMs = 0;
    setMediaTitle(QString());
    setMediaArtist(QString());
    setMediaArtSource(QString());
    setMediaTimestamp(QString());
    setMediaPlaying(false);
    setMprisVisible(false);
    updateMprisPlaybackTicker();
}

void ValenzBridge::updateMprisPlaybackTicker()
{
    if (!m_mprisPlaybackTimer)
        return;

    const bool shouldRun = m_mprisVisible && m_mediaPlaying && !m_mprisServiceName.isEmpty();

    if (shouldRun && !m_mprisPlaybackTimer->isActive())
        m_mprisPlaybackTimer->start();
    else if (!shouldRun && m_mprisPlaybackTimer->isActive())
        m_mprisPlaybackTimer->stop();
}

void ValenzBridge::updateMprisTimestampFromTicker()
{
    if (!m_mprisVisible || !m_mediaPlaying || m_mprisServiceName.isEmpty())
        return;

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (m_mprisLastPositionEpochMs <= 0 || nowMs <= m_mprisLastPositionEpochMs)
        return;

    const qint64 elapsedUs = (nowMs - m_mprisLastPositionEpochMs) * 1000;
    qint64 projectedUs = qMax<qint64>(0, m_mprisLastPositionUs + elapsedUs);

    if (m_mprisTrackLengthUs > 0)
        projectedUs = qMin(projectedUs, m_mprisTrackLengthUs);

    if (projectedUs == m_mprisPositionUs)
        return;

    const QString previousTimestamp = m_mediaTimestamp;
    m_mprisPositionUs = projectedUs;

    const QString timestamp = formatMprisTimestamp(m_mprisPositionUs, m_mprisTrackLengthUs);
    if (timestamp != previousTimestamp)
        setMediaTimestamp(timestamp);
}


void ValenzBridge::onMprisPropertiesChanged(const QString &interfaceName,
                                            const QVariantMap &changedProperties,
                                            const QStringList &invalidatedProperties)
{
    if (interfaceName != QString::fromLatin1(kMprisPlayerInterface))
        return;
    refreshMprisState();
}

void ValenzBridge::onMprisNameOwnerChanged(const QString &serviceName,
                                           const QString &oldOwner,
                                           const QString &newOwner)
{
    if (!serviceName.startsWith(QString::fromLatin1(kMprisServicePrefix)))
        return;
    refreshMprisState();
}

QStringList ValenzBridge::mprisServiceNames() const
{
    QDBusConnectionInterface *busInterface = QDBusConnection::sessionBus().interface();
    if (!busInterface)
    {
        return {};
    }

    const QDBusReply<QStringList> namesReply = busInterface->registeredServiceNames();
    if (!namesReply.isValid())
    {
        return {};
    }

    QStringList services;
    const QStringList allServices = namesReply.value();

    for (const QString &service : allServices)
    {
        if (service.startsWith(QString::fromLatin1(kMprisServicePrefix)))
            services << service;
    }
    return services;
}

QVariantMap ValenzBridge::mprisPlayerProperties(const QString &serviceName) const
{
    if (serviceName.isEmpty())
        return {};

    QDBusInterface propertiesIface(serviceName,
                                   QString::fromLatin1(kMprisObjectPath),
                                   QString::fromLatin1(kDbusPropertiesInterface),
                                   QDBusConnection::sessionBus());

    const QDBusReply<QVariantMap> reply = propertiesIface.call(QStringLiteral("GetAll"),
                                                                QString::fromLatin1(kMprisPlayerInterface));

    if (!reply.isValid())
    {
        return {};
    }

    return reply.value();
}

QVariantMap ValenzBridge::mprisPlayerMetadata(const QString &serviceName) const
{
    if (serviceName.isEmpty())
        return {};

    QDBusInterface propertiesIface(serviceName,
                                   QString::fromLatin1(kMprisObjectPath),
                                   QString::fromLatin1(kDbusPropertiesInterface),
                                   QDBusConnection::sessionBus());

    const QDBusReply<QDBusVariant> reply = propertiesIface.call(QStringLiteral("Get"),
                                                                QString::fromLatin1(kMprisPlayerInterface),
                                                                QStringLiteral("Metadata"));

    if (!reply.isValid())
    {
        return {};
    }

    return variantToVariantMap(reply.value().variant());
}

QString ValenzBridge::preferredMprisService() const
{
    const QStringList services = mprisServiceNames();
    if (services.isEmpty())
        return {};

    QString fallbackService = services.constFirst();

    for (const QString &service : services)
    {
        const QVariantMap properties = mprisPlayerProperties(service);
        const QString playbackStatus = unwrapMprisVariant(properties.value(QStringLiteral("PlaybackStatus"))).toString().trimmed();
        QVariantMap metadata = mprisPlayerMetadata(service);
        if (metadata.isEmpty())
            metadata = variantToVariantMap(properties.value(QStringLiteral("Metadata")));
        const QString title = unwrapMprisVariant(metadata.value(QStringLiteral("xesam:title"))).toString().trimmed();
        if (playbackStatus.compare(QStringLiteral("Playing"), Qt::CaseInsensitive) == 0)
        {
            return service;
        }

        if (fallbackService.isEmpty() && !properties.isEmpty())
            fallbackService = service;
    }    return fallbackService;
}

qint64 ValenzBridge::mprisPlayerPositionUs(const QString &serviceName) const
{
    if (serviceName.isEmpty())
        return 0;

    QDBusInterface propertiesIface(serviceName,
                                   QString::fromLatin1(kMprisObjectPath),
                                   QString::fromLatin1(kDbusPropertiesInterface),
                                   QDBusConnection::sessionBus());

    const QDBusReply<QDBusVariant> reply = propertiesIface.call(QStringLiteral("Get"),
                                                                QString::fromLatin1(kMprisPlayerInterface),
                                                                QStringLiteral("Position"));

    if (!reply.isValid())
        return 0;

    return qMax<qint64>(0, reply.value().variant().toLongLong());
}

QString ValenzBridge::formatMprisTimeUs(qint64 microseconds) const
{
    const qint64 totalSeconds = qMax<qint64>(0, microseconds / 1000000);
    const qint64 minutes = totalSeconds / 60;
    const qint64 seconds = totalSeconds % 60;

    return QStringLiteral("%1:%2").arg(minutes).arg(seconds, 2, 10, QLatin1Char('0'));
}

QString ValenzBridge::formatMprisTimestamp(qint64 positionUs, qint64 lengthUs) const
{
    const QString left = formatMprisTimeUs(positionUs);
    const QString right = lengthUs > 0 ? formatMprisTimeUs(lengthUs) : QStringLiteral("--:--");
    return left + QStringLiteral(" / ") + right;
}

bool ValenzBridge::invokeMprisPlayerMethod(const QString &method)
{
    if (m_mprisServiceName.isEmpty())
        refreshMprisState();

    if (m_mprisServiceName.isEmpty())
        return false;

    QDBusInterface playerIface(m_mprisServiceName,
                               QString::fromLatin1(kMprisObjectPath),
                               QString::fromLatin1(kMprisPlayerInterface),
                               QDBusConnection::sessionBus());
    const QDBusMessage result = playerIface.call(method);
    if (result.type() == QDBusMessage::ErrorMessage)
    {
        return false;
    }

    return true;
}

void ValenzBridge::refreshMprisState()
{
    const QString serviceName = preferredMprisService();
    if (serviceName.isEmpty())
    {        clearMprisPropertiesSubscription();
        clearMprisState();
        return;
    }

    updateMprisPropertiesSubscription(serviceName);

    const QVariantMap playerProperties = mprisPlayerProperties(serviceName);
    if (playerProperties.isEmpty())
    {
        clearMprisState();
        return;
    }

    m_mprisServiceName = serviceName;

    const QString playbackStatus = unwrapMprisVariant(playerProperties.value(QStringLiteral("PlaybackStatus"))).toString().trimmed();
    const bool isPlaying = playbackStatus.compare(QStringLiteral("Playing"), Qt::CaseInsensitive) == 0;

    QVariantMap metadata = mprisPlayerMetadata(serviceName);
    if (metadata.isEmpty())
        metadata = variantToVariantMap(playerProperties.value(QStringLiteral("Metadata")));

    const QString title = unwrapMprisVariant(metadata.value(QStringLiteral("xesam:title"))).toString().trimmed();

    const QStringList artistList = variantToStringList(metadata.value(QStringLiteral("xesam:artist")));
    const QString artist = artistList.join(QStringLiteral(", "));

    const QString artSource = unwrapMprisVariant(metadata.value(QStringLiteral("mpris:artUrl"))).toString().trimmed();

    m_mprisTrackLengthUs = qMax<qint64>(0, unwrapMprisVariant(metadata.value(QStringLiteral("mpris:length"))).toLongLong());
    m_mprisPositionUs = mprisPlayerPositionUs(serviceName);

    if (m_mprisTrackLengthUs > 0)
        m_mprisPositionUs = qMin(m_mprisPositionUs, m_mprisTrackLengthUs);

    m_mprisLastPositionUs = m_mprisPositionUs;
    m_mprisLastPositionEpochMs = QDateTime::currentMSecsSinceEpoch();

    const QString timestamp = formatMprisTimestamp(m_mprisPositionUs, m_mprisTrackLengthUs);
    setMediaTitle(title);
    setMediaArtist(artist);
    setMediaArtSource(artSource);
    setMediaTimestamp(timestamp);
    setMediaPlaying(isPlaying);
    setMprisVisible(true);
    updateMprisPlaybackTicker();
}

void ValenzBridge::mediaPreviousTrack()
{
    if (!invokeMprisPlayerMethod(QStringLiteral("Previous")))
        return;

    trace(QStringLiteral("mpris"), QStringLiteral("previous_track"));
    refreshMprisState();
}

void ValenzBridge::mediaTogglePlayPause()
{
    if (!invokeMprisPlayerMethod(QStringLiteral("PlayPause")))
        return;

    trace(QStringLiteral("mpris"), QStringLiteral("toggle_play_pause"));
    refreshMprisState();
}

void ValenzBridge::mediaNextTrack()
{
    if (!invokeMprisPlayerMethod(QStringLiteral("Next")))
        return;

    trace(QStringLiteral("mpris"), QStringLiteral("next_track"));
    refreshMprisState();
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
    userSettings.remove("Window/focusedWindowTitle");
    userSettings.remove("window/title");

    userSettings.sync();
    m_focusedWindowTitle.clear();
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
