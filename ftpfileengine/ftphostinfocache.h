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

#ifndef FTPHOSTINFOCACHE_H
#define FTPHOSTINFOCACHE_H

#include <QMap>
#include <QString>
#include <QStringList>

#include <QObject>

class FtpHostInfoCache : public QObject
{
    Q_OBJECT
public:
    explicit FtpHostInfoCache(QObject *parent = 0);
    ~FtpHostInfoCache();

    enum { Password = 0, Port = 1, TransferMode = 2, Encoding = 3 };

    QStringList hostInfo(const QString& host, const QString &userName);

    QString password(const QString& host, const QString& userName)
    {
        QStringList info(hostInfo(host, userName));

        if (info.isEmpty())
            return QString();

        return info.at(Password);
    }

    int port(const QString& host, const QString& userName)
    {
        QStringList info(hostInfo(host, userName));

        if (info.isEmpty())
            return 21;

        return info.at(Port).toInt();
    }

    QString transferMode(const QString& host, const QString& userName)
    {
        QStringList info(hostInfo(host, userName));

        if (info.isEmpty())
            return "Passive";

        return info.at(TransferMode);
    }

    QString encoding(const QString& host, const QString& userName)
    {
        QStringList info(hostInfo(host, userName));

        if (info.isEmpty())
            return "System";

        return info.at(Encoding);
    }

    void addHostInfo(const QString& host, const QString& userName,
                     const QString& password, const QString& port,
                     const QString& transferMode, const QString& encoding);

    void addHostInfo(const QString& host, const QString& userName,
                     const QString& password, int port,
                     const QString& transferMode, const QString& encoding);

private:
    static QMap<QString, QStringList> _hostMap;
};

#endif // FTPHOSTINFOCACHE_H
