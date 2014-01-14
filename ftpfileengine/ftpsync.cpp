#include <QtCore>

#include "ftpsync.h"

FtpSync::FtpSync(QObject *parent)
    : QThread(parent)
    , _ftpDone(false)
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

void FtpSync::wait()
{
    while(!_ftpDone)
    {
        QThread::msleep(10);
        qApp->processEvents();
    }

    _ftpDone = false;
}

void FtpSync::ftpDone(bool error)
{
    Q_UNUSED(error);

    _ftpDone = true;
}
