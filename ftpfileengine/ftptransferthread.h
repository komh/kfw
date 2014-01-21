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
