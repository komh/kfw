#include <QFtp>
#include <QDebug>

#include "ftpfileengine.h"

#include "ftptransferthread.h"

FtpTransferThread::FtpTransferThread(FtpFileEngine* engine,
                                     QIODevice::OpenMode openMode,
                                     QObject *parent) :
    QThread(parent)
  , _engine(engine)
  , _openMode(openMode)
{
}

void FtpTransferThread::run()
{
    // Use a local event loop. The event loop of the thread itself seems to be
    // blocked as well when a main thread is blocked
    QEventLoop loop;
    QFtp ftp;

    connect(&ftp, SIGNAL(done(bool)), &loop, SLOT(quit()));

    ftp.connectToHost(_engine->_url.host(), _engine->_port);
    ftp.login(_engine->_userName, _engine->_password);

    if (_openMode & QIODevice::ReadOnly)
        ftp.get(_engine->_path, &_engine->_fileBuffer);
    else if (_openMode & QIODevice::WriteOnly)
        ftp.put(&_engine->_fileBuffer, _engine->_path);

    ftp.close();

    loop.exec();
}
