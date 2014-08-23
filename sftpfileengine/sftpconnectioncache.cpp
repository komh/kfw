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

#include "sftpconnectioncache.h"

#include <QUrl>

#ifdef Q_OS_OS2
#define closesocket ::close
#elif defined(Q_OS_WIN32)
#include <winsock2.h>
#endif

#include <libssh2.h>
#include <libssh2_sftp.h>

SFtpConnectionCache::SFtpConnectionCache()
{
#ifdef Q_OS_WIN32
        WSADATA wsadata;

        WSAStartup(MAKEWORD(2,2), &wsadata);
#endif

        libssh2_init(0);
}

SFtpConnectionCache::~SFtpConnectionCache()
{
    closeAll();

    libssh2_exit();
}

SFtpConnectionCache *SFtpConnectionCache::getInstance()
{
    static SFtpConnectionCache cache;

    return &cache;
}

const SFtpConnectionCache::SFtpConnection *
SFtpConnectionCache::findConnection(const QUrl &url) const
{
    QString key(getKey(url));

    if (_connectionMap.contains(key))
        return _connectionMap.value(key);

    return 0;
}

void SFtpConnectionCache::addConnection(const QUrl &url,
                                        int sock,
                                        LIBSSH2_SESSION *session,
                                        LIBSSH2_SFTP *sftp_session)
{
    SFtpConnection *conn = new SFtpConnection;
    conn->sock = sock;
    conn->session = session;
    conn->sftp_session = sftp_session;

    _connectionMap.insert(getKey(url), conn);
}

static void freeConnection(const SFtpConnectionCache::SFtpConnection *conn)
{
    libssh2_sftp_shutdown(conn->sftp_session);
    libssh2_session_disconnect(conn->session, "Normal Shutdown");
    libssh2_session_free(conn->session);
    closesocket(conn->sock);

    delete(conn);
}

void SFtpConnectionCache::closeConnection(const QUrl &url)
{
    QString key(getKey(url));

    if (_connectionMap.contains(key))
    {
        freeConnection(findConnection(url));

        _connectionMap.remove(key);
    }
}

void SFtpConnectionCache::closeAll()
{
    QMapIterator<QString, const SFtpConnection *> it(_connectionMap);

    while (it.hasNext())
    {
        it.next();

        freeConnection(it.value());
    }

    _connectionMap.clear();
}

QString SFtpConnectionCache::getKey(const QUrl &url) const
{
    QString key;

    key.append(url.userName());
    key.append("@");
    key.append(url.host());
    key.append(":");
    key.append(QString::number(url.port()));

    return key;
}
