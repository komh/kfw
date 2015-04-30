/****************************************************************************
**
** FtpPool, pool for QFtp instances
**
** Copyright (C) 2015 by KO Myung-Hun
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

#ifndef FTPPOOL_H
#define FTPPOOL_H

#include <QObject>
#include <QThread>
#include <QList>
#include <QFtp>

class FtpPool : public QObject
{
    Q_OBJECT
public:
    static FtpPool *getInstance();

    QFtp *createFtp();
    void destroyFtp(QFtp *ftp);

    void destroyFtpPool();

private:
    explicit FtpPool(QObject *parent = 0);
    ~FtpPool();

private:
    QThread _thread;
    QList<QFtp *> _ftpList;
};

#endif // FTPPOOL_H
