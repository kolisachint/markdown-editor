# Sources and settings shared by the app and its tests, so a platform is added
# in one place. The including .pro sets SRC_ROOT to the top of the checkout
# first, because qmake resolves paths here against the build directory and
# tests/tests.pro sits one level below it.
#
# Linux reads the XDG portal and the omarchy theme from
# platformtheme_linux.cpp; every other OS compiles platformtheme.cpp, which
# answers the same questions from Qt's style hints. A new desktop only has to
# add a platformtheme_<os>.cpp here and guard the matching block in
# platformtheme.cpp the way Linux's is guarded.

QT += core gui widgets printsupport qml quick quickcontrols2 quickdialogs2

CONFIG += c++17
TEMPLATE = app

INCLUDEPATH += $$SRC_ROOT/src

HEADERS += \
    $$SRC_ROOT/src/backend.h \
    $$SRC_ROOT/src/markdownhighlighter.h \
    $$SRC_ROOT/src/platformtheme.h \
    $$SRC_ROOT/src/systemtheme.h

SOURCES += \
    $$SRC_ROOT/src/backend.cpp \
    $$SRC_ROOT/src/markdownhighlighter.cpp \
    $$SRC_ROOT/src/platformtheme.cpp \
    $$SRC_ROOT/src/systemtheme.cpp

linux {
    QT += dbus
    SOURCES += $$SRC_ROOT/src/platformtheme_linux.cpp
}

# Qt from Homebrew is built against whichever SDK the machine had, so its own
# version check warns on any machine with a newer one installed.
macx: CONFIG += sdk_no_version_check

# The macOS mkspec wraps every TEMPLATE = app in a bundle of its own, which
# would double up with the one bin/build-macos assembles and would hide the
# test binary from bin/test. Both want the plain executable.
macx: CONFIG -= app_bundle

RESOURCES += $$SRC_ROOT/src/resources.qrc
