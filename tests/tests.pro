SRC_ROOT = $$_PRO_FILE_PWD_/..

TARGET = tst_omawrite
CONFIG += testcase
QT += testlib

include(../omawrite.pri)

SOURCES += $$SRC_ROOT/tests/tst_omawrite.cpp
