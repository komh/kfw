#ifndef FTPBUFFER_H
#define FTPBUFFER_H

#include <QIODevice>
#include <QBuffer>

class FtpBuffer : public QIODevice
{
    Q_OBJECT
public:
    explicit FtpBuffer(QObject *parent = 0);

    bool atEnd() const;
    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool canReadLine() const;
    void close();
    bool isSequential() const;
    bool open(OpenMode mode);
    qint64 pos() const;
    qint64 readPos() const { return _readPos; }
    bool reset();
    bool seek(qint64 pos);
    qint64 size() const;
    bool waitForBytesWritten(int msecs);
    bool waitForReadyRead(int msecs);

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    QBuffer _buffer;

    qint64 _readPos;
};

#endif // FTPBUFFER_H
