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

#ifndef SERVERINFO_H
#define SERVERINFO_H

#include <QString>
#include <QStringList>

class ServerInfo
{
public:
    enum Protocol {Ftp = 0};
    enum TransferMode {Passive = 0, Active = 1};
    enum Encoding {System = 0, Utf8 = 1};

    ServerInfo();

    QString name() const;
    QString host() const;
    int port() const;
    Protocol protocol() const;
    TransferMode transferMode() const;
    Encoding encoding() const;
    bool isAnonymous() const;
    QString userName() const;
    QString password() const;
    QString directory() const;

    void setName(const QString& name);
    void setHost(const QString& host);
    void setPort(int port);
    void setProtocol(Protocol protocol);
    void setProtocol(const QString& protocol);
    void setTransferMode(TransferMode mode);
    void setTransferMode(const QString& mode);
    void setEncoding(Encoding encoding);
    void setEncoding(const QString& encoding);
    void setAnonymous(bool isAnonymous);
    void setUserName(const QString& userName);
    void setPassword(const QString& password);
    void setDirectory(const QString& directory);

    QString locationUrl() const;

    static QString protocolText(Protocol protocol);
    static QStringList protocolList();

    static QString transferModeText(TransferMode mode);
    static QStringList transferModeList();

    static QString encodingText(Encoding encoding);
    static QStringList encodingList();

private:
    QString _name;
    QString _host;
    int _port;
    Protocol _protocol;
    TransferMode _transferMode;
    Encoding _encoding;
    bool _isAnonymous;
    QString _userName;
    QString _password;
    QString _directory;
};

#endif // SERVERINFO_H
