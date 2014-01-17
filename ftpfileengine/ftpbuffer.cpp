#include <QDebug>

#include "ftpbuffer.h"

FtpBuffer::FtpBuffer(QObject *parent) :
    QIODevice(parent)
  , _readPos(0)
{
}

bool FtpBuffer::atEnd() const
{
    qDebug() << "atEnd()";

    return readPos() >= _buffer.size();
}

qint64 FtpBuffer::bytesAvailable() const
{
    qDebug() << "bytesAvailable()";

    return _buffer.size() - readPos();
}

qint64 FtpBuffer::bytesToWrite() const
{
    qDebug() << "bytesToWrite()";

    return bytesAvailable();
}

bool FtpBuffer::canReadLine() const
{
    qDebug() << "canReadLine()";

    return _buffer.canReadLine();
}

void FtpBuffer::close()
{
    qDebug() << "close()";

    _buffer.close();

    QIODevice::close();
}

bool FtpBuffer::isSequential() const
{
    qDebug() << "isSequential()";

    return _buffer.isSequential();
}

bool FtpBuffer::open(OpenMode mode)
{
    qDebug() << "open()" << mode;

    return QIODevice::open(mode) && _buffer.open(mode);
}

qint64 FtpBuffer::pos() const
{
    qDebug() << "pos()";

    return _buffer.pos();
}

bool FtpBuffer::reset()
{
    qDebug() << "reset()";

    return QIODevice::reset() && _buffer.reset();
}

bool FtpBuffer::seek(qint64 pos)
{
    qDebug() << "seek()" << pos;

    return _buffer.seek(pos);
}

qint64 FtpBuffer::size() const
{
    qDebug() << "size()";

    return _buffer.size();
}

bool FtpBuffer::waitForBytesWritten(int msecs)
{
    qDebug() << "waitForBytesWritten()" << msecs;

    return _buffer.waitForBytesWritten(msecs);
}

bool FtpBuffer::waitForReadyRead(int msecs)
{
    qDebug() << "waitForReadyRead()" << msecs;

    return _buffer.waitForReadyRead(msecs);
}

qint64 FtpBuffer::readData(char *data, qint64 maxlen)
{
    qDebug() << "readData()" << maxlen;

    qint64 len = qMin(maxlen, bytesAvailable());

    if (len > 0)
    {
        memcpy(data, _buffer.buffer().data() + readPos(), len);

        _readPos += len;
    }

    return len;
}

qint64 FtpBuffer::writeData(const char *data, qint64 len)
{
    qDebug() << "writeData()" << len;

    return _buffer.write(data, len);
}
