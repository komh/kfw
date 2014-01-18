#ifndef FTPTRANSFERTHREAD_H
#define FTPTRANSFERTHREAD_H

#include <QThread>
#include <QIODevice>

class FtpFileEngine;

class FtpTransferThread : public QThread
{
    Q_OBJECT
public:
    explicit FtpTransferThread(FtpFileEngine* engine,
                               QIODevice::OpenMode openMode,
                               QObject *parent = 0);

protected:
    void run();

private:
    FtpFileEngine* _engine;
    QIODevice::OpenMode _openMode;
};

#endif // FTPTRANSFERTHREAD_H
