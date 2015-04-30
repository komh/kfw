/****************************************************************************
**
** FtpConnectionCache, a cache for FTP connections
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard
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

#include "ftpconnectioncache.h"

#include <QUrl>
#include <QDebug>

#include "ftpsync.h"

#include "ftppool.h"

FtpConnectionCache::FtpConnectionCache()
{
}

FtpConnectionCache::~FtpConnectionCache()
{
    // QEventLoop cannot be used here because QApplication already
    // destructed. So just delete QFtp* instances

    qDeleteAll(_connectionMap);
}

FtpConnectionCache *FtpConnectionCache::getInstance()
{
    static FtpConnectionCache cache;

    return &cache;
}

QFtp *FtpConnectionCache::findConnection(const QUrl &url) const
{
    QString key(getKey(url));

    if (_connectionMap.contains(key))
        return _connectionMap.value(key);

    return 0;
}

void FtpConnectionCache::addConnection(const QUrl &url, QFtp *ftp)
{
    QString key(getKey(url));

    _connectionMap.insert(key, ftp);
}

void FtpConnectionCache::closeConnection(const QUrl &url)
{
    QString key(getKey(url));

    if (_connectionMap.contains(key))
    {
        QFtp* ftp = findConnection(url);

        FtpSync ftpSync(ftp);

        ftp->close();
        ftpSync.wait();

        FtpPool::getInstance()->destroyFtp(ftp);

        _connectionMap.remove(key);
    }
}

void FtpConnectionCache::closeAll()
{
    QMapIterator<QString, QFtp*> it(_connectionMap);

    while (it.hasNext())
    {
        it.next();

        QFtp* ftp(it.value());

        FtpSync ftpSync(ftp);

        ftp->close();

        qDebug() << "closeAll()" << it.key() <<
        ftpSync.wait();

        FtpPool::getInstance()->destroyFtp(ftp);
    }

    _connectionMap.clear();
}

QString FtpConnectionCache::getKey(const QUrl &url) const
{
    QString key;

    key.append(url.userName());
    key.append("@");
    key.append(url.host());
    key.append(":");
    key.append(QString::number(url.port()));

    return key;
}
