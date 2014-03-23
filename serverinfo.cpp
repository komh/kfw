/****************************************************************************
**
** ServerInfo, a class containg a server information
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

#include "serverinfo.h"

#include <QObject>
#include <QUrl>

ServerInfo::ServerInfo()
{
}

QString ServerInfo::name() const
{
    return _name;
}

QString ServerInfo::host() const
{
    return _host;
}

int ServerInfo::port() const
{
    return _port;
}

ServerInfo::Protocol ServerInfo::protocol() const
{
    return _protocol;
}

ServerInfo::TransferMode ServerInfo::transferMode() const
{
    return _transferMode;
}

ServerInfo::Encoding ServerInfo::encoding() const
{
    return _encoding;
}

bool ServerInfo::isAnonymous() const
{
    return _isAnonymous;
}

QString ServerInfo::userName() const
{
    return _userName;
}

QString ServerInfo::password() const
{
    return _password;
}

QString ServerInfo::directory() const
{
    return _directory;
}

void ServerInfo::setName(const QString &name)
{
    _name = name;
}

void ServerInfo::setHost(const QString &host)
{
    _host = host;
}

void ServerInfo::setPort(int port)
{
    _port = port;
}

void ServerInfo::setProtocol(ServerInfo::Protocol protocol)
{
    _protocol = protocol;
}

void ServerInfo::setProtocol(const QString &protocol)
{
    int index = protocolList().indexOf(protocol);

    if (index == -1)
        return;

    setProtocol(static_cast<Protocol>(index));
}

void ServerInfo::setTransferMode(ServerInfo::TransferMode mode)
{
    _transferMode = mode;
}

void ServerInfo::setTransferMode(const QString &mode)
{
    int index = transferModeList().indexOf(mode);

    if (index == -1)
        return;

    setTransferMode(static_cast<TransferMode>(index));
}

void ServerInfo::setEncoding(ServerInfo::Encoding encoding)
{
    _encoding = encoding;
}

void ServerInfo::setEncoding(const QString &encoding)
{
    int index = encodingList().indexOf(encoding);

    if (index == -1)
        return;

    setEncoding(static_cast<Encoding>(index));
}

void ServerInfo::setAnonymous(bool isAnonymous)
{
    _isAnonymous = isAnonymous;
}

void ServerInfo::setUserName(const QString &userName)
{
    _userName = userName;
}

void ServerInfo::setPassword(const QString &password)
{
    _password = password;
}

void ServerInfo::setDirectory(const QString &directory)
{
    _directory = directory;
}

QString ServerInfo::locationUrl() const
{
    QUrl url;

    url.setScheme(protocolText(protocol()).toLower());
    url.setHost(host());

    if (!isAnonymous() && userName() != "anonymous")
    {
        url.setUserName(userName());

        if (!password().isEmpty())
            url.setPassword(password());
    }

    if (port() != 21)
        url.setPort(port());

    url.setPath(directory());

    url.setQueryItems(QList<QPair<QString, QString> >()
                      << QPair<QString, QString>
                            ("transfermode", transferModeText(transferMode()))
                      << QPair<QString, QString>
                            ("encoding", encodingText(encoding())));

    return url.toString();
}

QString ServerInfo::protocolText(Protocol protocol)
{
    if (protocol == Ftp)
        return "FTP";

    return QString();
}

QStringList ServerInfo::protocolList()
{
    QStringList list;

    list << protocolText(Ftp);

    return list;
}

QString ServerInfo::transferModeText(TransferMode mode)
{
    if (mode == Passive)
        return "Passive";
    else if (mode == Active)
        return "Active";

    return QString();
}

QStringList ServerInfo::transferModeList()
{
    QStringList list;

    list << transferModeText(Passive);
    list << transferModeText(Active);

    return list;
}

QString ServerInfo::encodingText(Encoding encoding)
{
    if (encoding == System)
        return "System";
    else if (encoding == Utf8)
        return "UTF-8";

    return QString();
}

QStringList ServerInfo::encodingList()
{
    QStringList list;

    list << encodingText(System);
    list << encodingText(Utf8);

    return list;
}
