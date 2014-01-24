/****************************************************************************
**
** FtpTransferThread, class to execute QFtp in a separte thread
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

#ifndef FTPTRANSFERTHREAD_H
#define FTPTRANSFERTHREAD_H

#include <QThread>
#include <QIODevice>

class FtpFileEngine;
class QFtp;

class FtpTransferThread : public QThread
{
    Q_OBJECT
public:
    explicit FtpTransferThread(FtpFileEngine* engine,
                               QIODevice::OpenMode openMode,
                               QObject *parent = 0);

    void abort();

signals:
    void ftpAbort();
    void loopQuit();

protected:
    void run();

private:
    bool _aborting;

    FtpFileEngine* _engine;
    QIODevice::OpenMode _openMode;

private slots:
    void ftpCommandFinished(int id, bool error);
};

#endif // FTPTRANSFERTHREAD_H
