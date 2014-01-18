#ifndef FTPBUFFER_H
#define FTPBUFFER_H

#include <QIODevice>
#include <QBuffer>
#include <QMutex>
#include <QWaitCondition>

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
    bool reset();
    bool seek(qint64 pos);
    qint64 size() const;
    bool waitForBytesWritten(int msecs);
    bool waitForReadyRead(int msecs);

    bool  flush();
    qint64 readPos() const;
    void setSize(qint64 size) { _size = size; }

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    QBuffer _buffer;
    qint64 _basePos;
    qint64 _readPos;
    mutable QMutex _mutex;
    mutable QMutex _bufferLengthMutex;
    mutable QWaitCondition _bufferLengthCond;

    qint64 _size;

    qint64 dataLength() const;
    bool isEnd() const;
    qint64 totalSize() const;
};

#endif // FTPBUFFER_H
