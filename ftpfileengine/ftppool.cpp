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

#include "ftppool.h"

#include <QEventLoop>
#include <QTimer>

class FtpWorker : public QObject
{
    Q_OBJECT
public:
    FtpWorker(QFtp *ftp = 0, QObject *parent = 0)
        : QObject(parent), _ftp(ftp) {}

    QFtp *ftp() { return _ftp; }

signals:
    void finished();

public slots:
    void create()
    {
        _ftp = new QFtp;

        emit finished();
    }

    void destroy()
    {
        delete _ftp;

        emit finished();
    }

private:
    QFtp *_ftp;
};

#include "ftppool.moc"

FtpPool *FtpPool::getInstance()
{
    static FtpPool ftpPool;

    return &ftpPool;
}

QFtp *FtpPool::createFtp()
{
    QEventLoop loop;
    FtpWorker worker;

    worker.moveToThread(&_thread);
    connect(&worker, SIGNAL(finished()), &loop, SLOT(quit()));
    QTimer::singleShot(0, &worker, SLOT(create()));
    loop.exec();

    QFtp *ftp = worker.ftp();

    _ftpList.append(ftp);

    return ftp;
}

void FtpPool::destroyFtp(QFtp *ftp)
{
    QEventLoop loop;
    FtpWorker worker(ftp);

    worker.moveToThread(&_thread);
    connect(&worker, SIGNAL(finished()), &loop, SLOT(quit()));
    QTimer::singleShot(0, &worker, SLOT(destroy()));
    loop.exec();

    _ftpList.removeAll(ftp);
}

void FtpPool::destroyFtpPool()
{
    foreach(QFtp *ftp, _ftpList)
        destroyFtp(ftp);

    _ftpList.clear();

    _thread.quit();
    _thread.wait();
}

FtpPool::FtpPool(QObject *parent) : QObject(parent)
{
    _thread.start();
}

FtpPool::~FtpPool()
{
    /* Do nothing. All are done in destoryFtpPool() when ftp:///:closeall:
     * is issued */
}
