#-------------------------------------------------
#
# Project created by QtCreator 2014-04-23T14:27:11
#
#-------------------------------------------------

QT       -= gui

TARGET = orsa
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    libOrsa.cpp \
    vector.cpp \
    TestResult.cpp \
    TestRegistry.cpp \
    Test.cpp \
    SimpleString.cpp \
    orsa_model.cpp \
    numerics.cpp \
    matrix.cpp \
    homography_model.cpp \
    Failure.cpp \
    conditioning.cpp \
    ccmath_svd.cpp

HEADERS += \
    libOrsa.h \
    TestResult.h \
    TestRegistry.h \
    testing.h \
    TestHarness.h \
    Test.h \
    SimpleString.h \
    orsa_model.hpp \
    numerics.h \
    matrix.h \
    match.h \
    homography_model.hpp \
    Failure.h \
    conditioning.hpp
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

CONFIG(debug, debug|release) {
    win32-*{
        !contains(QMAKE_TARGET.arch, x86_64) {
            DESTDIR = build/debug
        }else{
            DESTDIR = build64/debug
        }
    }
} else {
    win32-*{
        !contains(QMAKE_TARGET.arch, x86_64) {
            DESTDIR = build/release
        }else{
            DESTDIR = build64/release
        }
    }
}
