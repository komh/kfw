/****************************************************************************
**
** FtpHostInfoCache, class to cache host informations of FTP servers
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

#include "ftphostinfocache.h"

QMap<QString, QStringList> FtpHostInfoCache::_hostMap;

FtpHostInfoCache::FtpHostInfoCache(QObject *parent) :
    QObject(parent)
{
}

FtpHostInfoCache::~FtpHostInfoCache()
{

}

QStringList FtpHostInfoCache::hostInfo(const QString& host,
                                       const QString& userName)
{
    QString key(userName);

    key.append("@");
    key.append(host);

    return _hostMap.value(key);
}

void FtpHostInfoCache::addHostInfo(const QString& host,
                                   const QString& userName,
                                   const QString& password,
                                   const QString& port)
{
    QString key(userName);

    key.append("@");
    key.append(host);

    _hostMap.insert(key, QStringList() << password << port);
}

void FtpHostInfoCache::addHostInfo(const QString& host,
                                   const QString& userName,
                                   const QString& password, int port)
{
    addHostInfo(host, userName, password, QString::number(port));
}
