#-------------------------------------------------
#
# Project created by QtCreator 2014-01-09T13:30:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kfw
TEMPLATE = app


SOURCES += main.cpp\
        kfilewizard.cpp \
    filesystemsortfilterproxymodel.cpp \
    entrytreeview.cpp \
    entrylistmodel.cpp

HEADERS  += kfilewizard.h \
    filesystemsortfilterproxymodel.h \
    entrytreeview.h \
    entrylistmodel.h

FORMS    += kfilewizard.ui
