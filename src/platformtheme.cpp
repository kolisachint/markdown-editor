#include "platformtheme.h"

#include <QGuiApplication>
#include <QStyleHints>

namespace PlatformTheme {

// The style-hint answers, kept in their own namespace so the Linux file can
// fall back to them after the portal fails to answer.
namespace qt {

bool darkMode(bool *known) {
    *known = false;

    const QStyleHints *hints = QGuiApplication::styleHints();
    if (!hints)
        return false;

    // On macOS Qt reads this from the effective NSAppearance, so the app
    // follows the system Appearance setting without a line of code here.
    switch (hints->colorScheme()) {
    case Qt::ColorScheme::Dark:
        *known = true;
        return true;
    case Qt::ColorScheme::Light:
        *known = true;
        return false;
    case Qt::ColorScheme::Unknown:
        break;
    }

    return false;
}

qreal textScale(bool *known) {
    // Only GNOME has a desktop-wide apparent text size, and Qt exposes no hint
    // for one, so everywhere else the app's own 12px is the baseline.
    Q_UNUSED(known);
    return 1.0;
}

} // namespace qt

Watcher::Watcher(QObject *parent) : QObject(parent) {
    // The system appearance is the one change every desktop can report and Qt
    // can hear about. A slot that takes no arguments is fine for a signal that
    // takes one, so the listeners can re-read everything from scratch. This is
    // compiled everywhere; only the portal call below is Linux-only.
    if (QStyleHints *hints = QGuiApplication::styleHints())
        connect(hints, &QStyleHints::colorSchemeChanged, this, &Watcher::changed);

    watchPortalSettings(this);
}

// Everything below is the whole implementation on every platform except Linux,
// which replaces it with platformtheme_linux.cpp. That file is only compiled
// there, so the guard is what keeps the two from colliding at link time — and
// macOS and Windows are left with what is written here, needing no code of
// their own.
#ifndef Q_OS_LINUX

bool palette(Palette *out) {
    Q_UNUSED(out);
    return false;
}

QStringList palettePaths() {
    return {};
}

void watchPortalSettings(Watcher *watcher) {
    Q_UNUSED(watcher);
}

bool darkMode(bool *known) {
    return qt::darkMode(known);
}

qreal textScale(bool *known) {
    return qt::textScale(known);
}

#endif // !Q_OS_LINUX

} // namespace PlatformTheme
