/****************************************************************************
**
** SFtpFileEngine, class for SFTP access
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard.
**
** $BEGIN_LICENSE$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $END_LICENSE$
**
****************************************************************************/

#ifndef SFTPFILEENGINE_H
#define SFTPFILEENGINE_H

#include <QtCore>
#include <QUrlInfo>

#include <libssh2.h>
#include <libssh2_sftp.h>

#include "../ftpfileengine/ftpfileinfocache.h"

class SFtpFileEngine : public QObject, public QAbstractFileEngine
{
    Q_OBJECT

public:
    explicit SFtpFileEngine(QObject* parent = 0);
    explicit SFtpFileEngine(const QString& fileName, QObject* parent = 0);
    ~SFtpFileEngine();

    bool atEnd() const;
    Iterator* beginEntryList(QDir::Filters filters,
                             const QStringList &filterNames);
    bool caseSensitive() const;
    bool close();
    bool copy(const QString &newName);
    QStringList entryList(QDir::Filters filters,
                          const QStringList &filterNames) const;
    QFile::FileError error() const;
    QString errorString() const;
    bool extension(Extension extension, const ExtensionOption *option,
                   ExtensionReturn *output);
    FileFlags fileFlags(FileFlags type) const;
    QString fileName(FileName file) const;
    QDateTime fileTime(FileTime time) const;
    bool flush();
    int handle() const;
    bool isRelativePath() const;
    bool isSequential() const;
    bool link(const QString &newName);
    uchar* map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
    bool mkdir(const QString &dirName, bool createParentDirectories) const;
    bool open(QIODevice::OpenMode openMode);
    QString owner(FileOwner owner) const;
    uint ownerId(FileOwner owner) const;
    qint64 pos() const;
    qint64 read(char *data, qint64 maxlen);
    qint64 readLine(char *data, qint64 maxlen);
    bool remove();
    bool rename(const QString &newName);
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const;
    bool seek(qint64 pos);
    void setFileName(const QString &file);
    bool setPermissions(uint perms);
    bool setSize(qint64 size);
    qint64 size() const;
    bool supportsExtension(Extension extension) const;
    bool unmap(uchar *ptr);
    qint64 write(const char *data, qint64 len);

private:
    QString _fileName;
    QAbstractFileEngine::FileFlags _fileFlags;
    QUrlInfo _urlInfo;

    QUrl _url;
    QString _userName;
    QString _password;
    int _port;
    QString _path;
    QString _encoding;

    QTextCodec* _textCodec;

    int _sock;
    LIBSSH2_SESSION *_session;
    LIBSSH2_SFTP *_sftp_session;
    LIBSSH2_SFTP_HANDLE *_sftp_handle;

    FtpFileInfoCache* _fileInfoCache;

    void initFromFileName(const QString& file = QString());
    void initSFtp();
    void readDir(const QString &dir);
    void refreshFileInfoCache();
    void refreshFileInfoCache(const QString &path);
    bool sftpConnect();
    bool sftpDisconnect();
    QString getCachePath(const QString &path, bool key = false) const;
};
#endif // SFTPFILEENGINE_H
