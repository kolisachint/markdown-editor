#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

// The one place the app asks the desktop how it looks. Every platform answers
// the same questions, so the callers stay free of platform checks.
//
// platformtheme.cpp holds the answers built from Qt's style hints alone, and
// platformtheme_linux.cpp holds the Linux ones, which read the XDG portal and
// the omarchy theme instead. qmake compiles the Linux file on Linux alone, so
// macOS and Windows get the Qt answers with no per-OS code of their own — and a
// new desktop that reports something better only has to add one
// platformtheme_<os>.cpp.
namespace PlatformTheme {

struct Palette {
    QString background;
    QString foreground;
    QString accent;
    QString selection;
    // "dark", "light", or empty when the desktop does not say.
    QString mode;
};

// Dark mode, and the desktop-wide apparent text size as a multiplier on the
// app's own 12px. Each sets *known when the desktop has actually expressed a
// preference, so a missing answer is distinguishable from an answer of "light"
// or "no scaling". Both fall back to a value of true / 1.0.
bool darkMode(bool *known);
qreal textScale(bool *known);

// The desktop's own colors, if it publishes them. Returns false when it does
// not, which leaves the app on its built-in palette.
bool palette(Palette *out);

// Files that change when the desktop's palette changes, so the caller can
// watch them. Empty when there is nothing to watch.
QStringList palettePaths();

// Emits changed() whenever the desktop reports a change: the system appearance
// on every platform, plus the XDG portal's settings on Linux. Parenting one of
// these to whoever owns the connection is all it takes to stay in step.
class Watcher : public QObject {
    Q_OBJECT

public:
    explicit Watcher(QObject *parent = nullptr);

signals:
    void changed();
};

// Wires the XDG portal's catch-all settings signal into `watcher`, for
// platforms that have one. A no-op everywhere else, which is why the portal is
// invisible to the callers above.
void watchPortalSettings(Watcher *watcher);

// The answers built from Qt's style hints alone, which each platform falls back
// to whenever its own desktop has nothing to say. On macOS Qt reads the system
// Appearance setting on its own, so these are the whole implementation there.
namespace qt {

bool darkMode(bool *known);
qreal textScale(bool *known);

} // namespace qt

} // namespace PlatformTheme
