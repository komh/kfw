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
    fileoperation/movefileworker.cpp \
    delayedmessagebox.cpp \
    pathcomp.cpp \
    entryview/entrytreeview.cpp \
    entryview/entrylistdelegate.cpp \
    entryview/entrylistmodel.cpp \
    fileiconprovider.cpp \
    dirtreeview.cpp \
    locationcompleter.cpp \
    connecttodialog.cpp \
    serverinfo.cpp \
    addressbookdialog.cpp \
    sharedmemory.cpp \
    simplecrypt.cpp \
    ftpfileengine/ftpconnectioncache.cpp \
    sftpfileengine/sftpfileengine.cpp \
    sftpfileengine/sftpfileenginehandler.cpp \
    sftpfileengine/sftpconnectioncache.cpp

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
    connecttodialog.h \
    serverinfo.h \
    addressbookdialog.h \
    sharedmemory.h \
    simplecrypt.h \
    ftpfileengine/ftpconnectioncache.h \
    sftpfileengine/sftpfileengine.h \
    sftpfileengine/sftpfileenginehandler.h \
    sftpfileengine/sftpconnectioncache.h

FORMS    += kfilewizard.ui \
    connecttodialog.ui \
    addressbookdialog.ui

TRANSLATIONS = translations/kfw_ko.ts

TRANSLATIONS_ALL = $${TRANSLATIONS} \
    translations/qt_ko.ts

lrelease.input = TRANSLATIONS_ALL
lrelease.output = ${OBJECTS_DIR}/${QMAKE_FILE_BASE}.qm
lrelease.commands = lrelease ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += lrelease

DEFINES += USE_FTP_CONNECTION_CACHE

os2 {
    LIBS += -lssh2 -lcrypto -lssl -lz
} win32 {
    LIBS += -lssh2 -lssl -lcrypto -lws2_32 -lgdi32 -lcrypt32
}
