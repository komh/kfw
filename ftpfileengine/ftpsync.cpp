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

#include <QtCore>

#include "ftpsync.h"

FtpSync::FtpSync(QObject *parent)
    : QThread(parent)
    , _ftpDone(false)
    , _ftpError(false)
{
}

FtpSync::FtpSync(QFtp *ftp, QObject *parent)
    : QThread(parent)
    , _ftpDone(false)
{
    setFtp(ftp);
}

FtpSync::~FtpSync()
{
}

void FtpSync::setFtp(QFtp* ftp)
{
    connect(ftp, SIGNAL(done(bool)), this, SLOT(ftpDone(bool)));
}

bool FtpSync::wait()
{
    // check if already done
    if (!_ftpDone)
        _loop.exec();

    _ftpDone = false;

    return !_ftpError;
}

void FtpSync::ftpDone(bool error)
{
    _ftpError = error;

    _ftpDone = true;

    _loop.quit();
}
