#include "systemtheme.h"

#include "platformtheme.h"

SystemTheme::SystemTheme(QObject *parent) : QObject(parent) {
    m_darkMode = detectDarkMode();
    m_textScale = detectTextScale();

    // The platform decides what counts as a change: the system appearance on
    // every desktop, the XDG portal's settings on Linux. Parented here so the
    // connection dies with this object, which is all that holds it.
    auto *watcher = new PlatformTheme::Watcher(this);
    connect(watcher, &PlatformTheme::Watcher::changed, this, &SystemTheme::refresh);
}

void SystemTheme::refresh() {
    setDarkMode(detectDarkMode());
    setTextScale(detectTextScale());
}

bool SystemTheme::detectDarkMode() const {
    bool known = false;

    const bool dark = PlatformTheme::darkMode(&known);
    if (known)
        return dark;

    return true;
}

qreal SystemTheme::detectTextScale() const {
    bool known = false;

    const qreal scale = PlatformTheme::textScale(&known);
    if (known)
        return scale;

    return 1.0;
}

void SystemTheme::setDarkMode(bool darkMode) {
    if (m_darkMode == darkMode)
        return;

    m_darkMode = darkMode;
    emit darkModeChanged(m_darkMode);
}

void SystemTheme::setTextScale(qreal textScale) {
    if (qFuzzyCompare(m_textScale, textScale))
        return;

    m_textScale = textScale;
    emit textScaleChanged(m_textScale);
}

