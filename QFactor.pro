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
    TOTP.cpp

HEADERS  += QFactorMain.h \
    TOTP.h

FORMS    += QFactorMain.ui
