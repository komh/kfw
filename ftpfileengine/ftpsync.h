/****************************************************************************
**
** FtpSync, helper class to syncrhonize the execution of QFtp
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

#ifndef FTPSYNC_H
#define FTPSYNC_H

#include <QThread>
#include <QFtp>
#include <QEventLoop>

class FtpSync : public QThread
{
    Q_OBJECT

public:
    explicit FtpSync(QObject* parent = 0);
    explicit FtpSync(QFtp *ftp, QObject* parent = 0 );
    ~FtpSync();

    void setFtp(QFtp *ftp);
    bool wait(int ms = -1);
    bool timedOut() const { return _ftpTimedOut; }

private slots:
    void ftpDone(bool error);

private:
    bool _ftpDone;
    bool _ftpError;
    bool _ftpTimedOut;

    QEventLoop _loop;
};

#endif // FTPSYNC_H
