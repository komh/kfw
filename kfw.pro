#-------------------------------------------------
#
# Project created by QtCreator 2014-01-09T13:30:04
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kfw
TEMPLATE = app


SOURCES += main.cpp\
        kfilewizard.cpp \
    filesystemsortfilterproxymodel.cpp \
    entrytreeview.cpp \
    entrylistmodel.cpp \
    ftpfileengine/ftpsync.cpp \
    ftpfileengine/ftpfileinfocache.cpp \
    ftpfileengine/ftpfileengineiterator.cpp \
    ftpfileengine/ftpfileenginehandler.cpp \
    ftpfileengine/ftpfileengine.cpp

HEADERS  += kfilewizard.h \
    filesystemsortfilterproxymodel.h \
    entrytreeview.h \
    entrylistmodel.h \
    ftpfileengine/ftpsync.h \
    ftpfileengine/ftpfileinfocache.h \
    ftpfileengine/ftpfileengineiterator.h \
    ftpfileengine/ftpfileenginehandler.h \
    ftpfileengine/ftpfileengine.h

FORMS    += kfilewizard.ui
