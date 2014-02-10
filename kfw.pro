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
    ftpfileengine/ftpsync.cpp \
    ftpfileengine/ftpfileinfocache.cpp \
    ftpfileengine/ftpfileengineiterator.cpp \
    ftpfileengine/ftpfileenginehandler.cpp \
    ftpfileengine/ftpfileengine.cpp \
    ftpfileengine/ftpbuffer.cpp \
    ftpfileengine/ftptransferthread.cpp \
    ftpfileengine/ftphostinfocache.cpp \
    urllistmimedata.cpp \
    fileoperation/fileoperation.cpp \
    fileoperation/abstractfileworker.cpp \
    fileoperation/copyfileworker.cpp \
    fileoperation/removefileworker.cpp \
    fileoperation/renamefileworker.cpp \
    fileoperation/movefileworker.cpp \
    delayedmessagebox.cpp \
    pathcomp.cpp \
    entryview/entrytreeview.cpp \
    entryview/entrylistdelegate.cpp \
    entryview/entrylistmodel.cpp \
    fileiconprovider.cpp \
    dirtreeview.cpp \
    locationcompleter.cpp \
    connecttodialog.cpp

HEADERS  += kfilewizard.h \
    filesystemsortfilterproxymodel.h \
    ftpfileengine/ftpsync.h \
    ftpfileengine/ftpfileinfocache.h \
    ftpfileengine/ftpfileengineiterator.h \
    ftpfileengine/ftpfileenginehandler.h \
    ftpfileengine/ftpfileengine.h \
    ftpfileengine/ftpbuffer.h \
    ftpfileengine/ftptransferthread.h \
    ftpfileengine/ftphostinfocache.h \
    urllistmimedata.h \
    fileoperation/fileoperation.h \
    fileoperation/abstractfileworker.h \
    fileoperation/copyfileworker.h \
    fileoperation/removefileworker.h \
    fileoperation/renamefileworker.h \
    fileoperation/movefileworker.h \
    delayedmessagebox.h \
    pathcomp.h \
    entryview/entrylistdelegate.h \
    entryview/entrylistmodel.h \
    entryview/entrytreeview.h \
    fileiconprovider.h \
    qttr.h \
    dirtreeview.h \
    locationcompleter.h \
    connecttodialog.h

FORMS    += kfilewizard.ui \
    connecttodialog.ui

TRANSLATIONS = translations/kfw_ko.ts
