/****************************************************************************
**
** FtpFileInfoCache, class to cache file informations of FTP servers
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

#ifndef FTPFILEINFOCACHE_H
#define FTPFILEINFOCACHE_H

#include <QtCore>

#include <QUrlInfo>

class FtpFileInfoCache
{
public:
    void addFileInfo(const QString& dir, const QUrlInfo& urlInfo);
    void removeFileInfo(const QString& dir, const QString& name);
    void removeFileInfo(const QString& path);
    void removeDirInfo(const QString& dir);
    void renameFileInfo(const QString& dir, const QString& name,
                        const QString &newDir, const QString& newName);
    void renameFileInfo(const QString& path, const QString& newPath);

    QUrlInfo findFileInfo(const QString& dir, const QString& name);
    QUrlInfo findFileInfo(const QString& path);

    typedef QList<QUrlInfo> QUrlInfoList;
    typedef QListIterator<QUrlInfo> QUrlInfoListIterator;

    QUrlInfoList findDirInfo(const QString& dir);

    static FtpFileInfoCache* getInstance();

private:
    explicit FtpFileInfoCache();
    ~FtpFileInfoCache();

    static QMultiMap<QString, QUrlInfo> _dirMultiMap;

    QString getCacheKey(const QString& dir, const QString &name = QString());
};

#endif // FTPFILEINFOCACHE_H
