/****************************************************************************
**
** SFtpConnectionCache, a cache for SFTP connections
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

#ifndef SFTPCONNECTIONCACHE_H
#define SFTPCONNECTIONCACHE_H

#include <QUrl>
#include <QMap>

#include <libssh2.h>
#include <libssh2_sftp.h>

class SFtpConnectionCache
{
public:
    static SFtpConnectionCache* getInstance();

    struct SFtpConnection
    {
        int sock;
        LIBSSH2_SESSION *session;
        LIBSSH2_SFTP *sftp_session;
    };

    const SFtpConnection *findConnection(const QUrl &url) const;
    void addConnection(const QUrl &url, int sock,
                       LIBSSH2_SESSION *session,
                       LIBSSH2_SFTP *sftp_session);
    void closeConnection(const QUrl &url);
    void closeAll();

private:
    SFtpConnectionCache();
    ~SFtpConnectionCache();

    QString getKey(const QUrl& url) const;

    QMap<QString, const SFtpConnection *> _connectionMap;
};

#endif // SFTPCONNECTIONCACHE_H
