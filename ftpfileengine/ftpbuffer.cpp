#include <QtCore>

#include "ftpbuffer.h"

FtpBuffer::FtpBuffer(QObject *parent) :
    QIODevice(parent)
  , _basePos(0)
  , _readPos(0)
  , _mutex(QMutex::Recursive)
  , _size(-1)
{
}

bool FtpBuffer::atEnd() const
{
    qDebug() << "atEnd()" << readPos() << size();

    return isEnd();
}

qint64 FtpBuffer::bytesAvailable() const
{
    qDebug() << "bytesAvailable()";
    qDebug() << "\t" << dataLength();

    return dataLength();
}

qint64 FtpBuffer::bytesToWrite() const
{
    qDebug() << "bytesToWrite()";
    qDebug() << "\t" << dataLength();

    return dataLength();
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

    // QFtp::put() checks atEnd() when it reaches EOF, but only on
    // non-sequential device. So without this, QFtp::put() does not call
    // atEnd() and waits infinitely.
    return !isEnd();
}

bool FtpBuffer::open(OpenMode mode)
{
    qDebug() << "open()" << mode;

    return QIODevice::open(mode) && _buffer.open(mode);
}

qint64 FtpBuffer::pos() const
{
    qDebug() << "pos()";

    return 0;
}

bool FtpBuffer::reset()
{
    qDebug() << "reset()";

    return false;
}

bool FtpBuffer::seek(qint64 pos)
{
    qDebug() << "seek()" << pos;

    return false;
}

qint64 FtpBuffer::size() const
{
    qDebug() << "size()";

    return totalSize();
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
    qDebug() << "readData()" << maxlen << dataLength()
                             << QThread::currentThreadId();

    _bufferLengthMutex.lock();

    while (!isEnd() && dataLength() == 0)
    {
        qDebug() << "readData() data empty";
        _bufferLengthCond.wait(&_bufferLengthMutex, 10);
    }

    _bufferLengthMutex.unlock();

    qint64 len = qMin(maxlen, dataLength());

    if (len > 0)
    {
        QMutexLocker locker(&_bufferLengthMutex);

        _mutex.lock();

        memcpy(data, _buffer.buffer().data() + _readPos, len);

        _readPos += len;

        if (dataLength() == 0)
        {
            _buffer.reset();
            _buffer.buffer().clear();

            _basePos += _readPos;
            _readPos = 0;
        }

        _mutex.unlock();

        _bufferLengthCond.wakeAll();
    }

    qDebug() << "readData() =" << len;
    return len;
}

qint64 FtpBuffer::writeData(const char *data, qint64 len)
{
    qDebug() << "writeData()" << len << _buffer.size()
                              << QThread::currentThreadId();

    QMutexLocker locker(&_bufferLengthMutex);

    _mutex.lock();

    qint64 result = _buffer.write(data, len);

    _mutex.unlock();

    _bufferLengthCond.wakeAll();

    qDebug() << "writeData() =" << result;

    return result;
}

bool FtpBuffer::flush()
{
    qDebug() << "flush()" << "waiting...";

    QMutexLocker locker(&_bufferLengthMutex);

    while (dataLength() > 0)
    {
        qDebug() << "flush() data not empty";
        _bufferLengthCond.wait(&_bufferLengthMutex, 10);
    }

    qDebug() << "flush()" << "completed!!!";

    return true;
}

qint64 FtpBuffer::readPos() const
{
    QMutexLocker locker(&_mutex);

    return _basePos + _readPos;
}

qint64 FtpBuffer::dataLength() const
{
    QMutexLocker locker(&_mutex);

    return _buffer.pos() - _readPos;
}

bool FtpBuffer::isEnd() const
{
    return readPos() >= totalSize();
}

qint64 FtpBuffer::totalSize() const
{
    QMutexLocker locker(&_mutex);

    return (_size == -1 ? (_basePos + _buffer.pos()) : _size);
}
