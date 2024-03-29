#-------------------------------------------------
#
# Project created by QtCreator 2014-10-15T13:01:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QFactor
TEMPLATE = app


SOURCES += main.cpp\
        QFactorMain.cpp \
    TOTP.cpp \
    NewTOTPDialog.cpp \
    TOTPUtil.cpp

HEADERS  += QFactorMain.h \
    TOTP.h \
    NewTOTPDialog.h \
    TOTPUtil.h

FORMS    += QFactorMain.ui \
    NewTOTPDialog.ui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += liboath
}

TRANSLATIONS += locale/en_US.ts locale/it_IT.ts
