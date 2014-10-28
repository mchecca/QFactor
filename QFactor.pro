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
    NewTOTPDialog.cpp

HEADERS  += QFactorMain.h \
    TOTP.h \
    NewTOTPDialog.h

FORMS    += QFactorMain.ui \
    NewTOTPDialog.ui

CONFIG += link_pkgconfig
PKGCONFIG += liboath

TRANSLATIONS += locale/en_US.ts
