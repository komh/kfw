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
