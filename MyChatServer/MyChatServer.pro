#-------------------------------------------------
#
# Project created by QtCreator 2023-02-27T09:38:45
#
#-------------------------------------------------

QT += core gui network
QT += multimedia
QT += multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyChatServer
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h \
    DataToSend.h

FORMS    += widget.ui
