#include "platformtheme.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QVariant>

// The Linux side of the theme seam: the XDG portal for dark mode and apparent
// text size, and the omarchy theme for the desktop's colors. qmake compiles
// this file on Linux only; every other platform uses platformtheme.cpp as is.

namespace {

QVariant unwrapVariant(QVariant value) {
    while (value.canConvert<QDBusVariant>())
        value = value.value<QDBusVariant>().variant();
    return value;
}

bool colorSchemeIsDark(const QVariant &value, bool *known) {
    bool ok = false;
    const uint scheme = unwrapVariant(value).toUInt(&ok);
    if (!ok)
        return false;

    if (scheme == 1) {
        *known = true;
        return true;
    }
    if (scheme == 2) {
        *known = true;
        return false;
    }

    return false;
}

// GNOME's text-scaling-factor is the desktop-wide "apparent text size" knob;
// omarchy drives it from `omarchy display text size`, anchored so the default
// 12px maps to 1.0. Ignore nonsense values and cap the range GNOME allows.
qreal sanitizedTextScale(const QVariant &value, bool *known) {
    bool ok = false;
    const qreal scale = unwrapVariant(value).toDouble(&ok);
    if (!ok || scale <= 0)
        return 1.0;

    *known = true;
    return qBound(0.5, scale, 3.0);
}

// Ask the desktop portal for a single setting, returning an invalid variant
// when the portal is missing or slow to answer; the short timeout keeps a
// stalled portal from holding up the GUI thread.
QVariant portalSetting(const QString &nameSpace, const QString &key) {
    const QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return {};

    QDBusMessage request = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.portal.Desktop"),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"),
        QStringLiteral("Read"));
    request << nameSpace << key;

    const QDBusReply<QDBusVariant> reply(bus.call(request, QDBus::Block, 150));
    if (!reply.isValid())
        return {};

    return reply.value().variant();
}

// Omarchy keeps its colors in a flat `key = "value"` TOML table; parsing that
// here costs a few lines and saves the app a TOML dependency.
QString omarchyColorsPath() {
    return QDir::homePath()
        + QStringLiteral("/.local/state/omarchy/current/theme/colors.toml");
}

QString unquote(QString value) {
    if (value.size() >= 2
        && ((value.front() == QLatin1Char('"') && value.back() == QLatin1Char('"'))
            || (value.front() == QLatin1Char('\'') && value.back() == QLatin1Char('\''))))
        return value.mid(1, value.size() - 2);
    return value;
}

void readOmarchyColors(PlatformTheme::Palette *out) {
    QFile file(omarchyColorsPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        const int equals = line.indexOf(QLatin1Char('='));
        if (equals < 0)
            continue;

        const QString key = line.left(equals).trimmed();
        const QString value = unquote(line.mid(equals + 1).trimmed());

        if (key == QStringLiteral("mode"))
            out->mode = value;
        else if (key == QStringLiteral("background"))
            out->background = value;
        else if (key == QStringLiteral("foreground"))
            out->foreground = value;
        else if (key == QStringLiteral("accent"))
            out->accent = value;
        else if (key == QStringLiteral("selection"))
            out->selection = value;
    }
}

} // namespace

namespace PlatformTheme {

bool darkMode(bool *known) {
    *known = false;

    const QVariant scheme = portalSetting(QStringLiteral("org.freedesktop.appearance"),
                                          QStringLiteral("color-scheme"));
    if (scheme.isValid()) {
        const bool dark = colorSchemeIsDark(scheme, known);
        if (*known)
            return dark;
    }

    // The portal answered with something this does not recognise, or with
    // nothing at all. Qt derived its answer from the same desktop, so it is
    // the one to fall back on.
    return qt::darkMode(known);
}

qreal textScale(bool *known) {
    *known = false;

    const QVariant factor = portalSetting(QStringLiteral("org.gnome.desktop.interface"),
                                          QStringLiteral("text-scaling-factor"));
    if (!factor.isValid())
        return qt::textScale(known);

    return sanitizedTextScale(factor, known);
}

bool palette(Palette *out) {
    if (!QFile::exists(omarchyColorsPath()))
        return false;

    *out = {};
    readOmarchyColors(out);
    return true;
}

QStringList palettePaths() {
    const QString current = QDir::homePath() + QStringLiteral("/.local/state/omarchy/current");
    const QString theme = current + QStringLiteral("/theme");
    const QString colors = theme + QStringLiteral("/colors.toml");

    QStringList paths;
    if (QDir(current).exists())
        paths << current;
    if (QDir(theme).exists())
        paths << theme;
    if (QFile::exists(colors))
        paths << colors;

    return paths;
}

void watchPortalSettings(Watcher *watcher) {
    // The portal pushes color-scheme and text-scaling-factor through this one
    // signal, and D-Bus lets a slot take fewer arguments than the message
    // carries, so one connection covers every setting the portal reports. The
    // listeners re-read whatever they care about, which keeps GNOME-specific
    // knowledge out of SystemTheme.
    QDBusConnection::sessionBus().connect(
        QString(),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"),
        QStringLiteral("SettingChanged"),
        watcher, SLOT(changed()));
}

} // namespace PlatformTheme
