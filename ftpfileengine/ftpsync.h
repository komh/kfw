#ifndef FTPSYNC_H
#define FTPSYNC_H

#include <QThread>
#include <QFtp>

class FtpSync : public QThread
{
    Q_OBJECT

public:
    explicit FtpSync(QObject* parent = 0);
    explicit FtpSync(QFtp *ftp, QObject* parent = 0 );
    ~FtpSync();

    void setFtp(QFtp *ftp);
    void wait();

private slots:
    void ftpDone(bool error);

private:
    bool _ftpDone;
};

#endif // FTPSYNC_H
