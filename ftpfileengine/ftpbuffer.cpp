/****************************************************************************
**
** FtpBuffer, class for FTP buffer
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard.
**
** $BEGIN_LICENSE$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $END_LICENSE$
**
****************************************************************************/

#include <QtCore>

#include "ftpbuffer.h"

FtpBuffer::FtpBuffer(QObject *parent) :
    QIODevice(parent)
  , _basePos(0)
  , _readPos(0)
  , _mutex(QMutex::Recursive)
  , _abort(false)
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
    bool result = !isEnd();

    qDebug() << "isSequential()" << result;

    return result;
}

bool FtpBuffer::open(OpenMode mode)
{
    qDebug() << "open()" << mode;

    _basePos = 0;
    _readPos = 0;
    _abort = false;

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
    qDebug() << "readData()" << maxlen << QThread::currentThreadId();

    qDebug() << "readData()"
             << "dataLength =" << dataLength()
             << "isEnd() = " << isEnd();
    while (!isEnd() && dataLength() == 0)
    {
        QMutexLocker locker(&_dataLengthMutex);
        qDebug() << "readData()" << "data empty" << dataLength();
        _dataLengthCond.wait(&_dataLengthMutex);
        qDebug() << "readData()" << "waken up" << dataLength();
    }

    qint64 len = qMin(maxlen, dataLength());

    if (len > 0)
    {
        QMutexLocker locker(&_dataLengthMutex);

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

        _dataLengthCond.wakeAll();
    }

    QMutexLocker locker(&_dataLengthMutex);

    qDebug() << "readData()" << "len =" << len << "_abort =" << _abort;
    return _abort ? 0 : len;
}

static const qint64 MAX_BUFFER_SIZE = 32 * 1024;

qint64 FtpBuffer::writeData(const char *data, qint64 len)
{
    qDebug() << "writeData()" << len << _buffer.size()
                              << QThread::currentThreadId();

    QMutexLocker locker(&_dataLengthMutex);

    while (!_abort && _buffer.size() > MAX_BUFFER_SIZE)
    {
        qDebug() << "writeData()" << "buffer too large" << _buffer.size();
        _dataLengthCond.wait(&_dataLengthMutex);
        qDebug() << "writeData()" << "waken up" << _buffer.size();
    }

    if (_abort)
        return 0;

    _mutex.lock();

    qint64 result = _buffer.write(data, len);

    _mutex.unlock();

    _dataLengthCond.wakeAll();

    qDebug() << "writeData()" << result;

    return result;
}

void FtpBuffer::abort()
{
    QMutexLocker locker(&_dataLengthMutex);

    _abort = true;

    _dataLengthCond.wakeAll();
}

bool FtpBuffer::flush()
{
    qDebug() << "flush()" << "waiting...";

    QMutexLocker locker(&_dataLengthMutex);

    while (!_abort && dataLength() > 0)
    {
        qDebug() << "flush()" << "data not empty" << dataLength();
        _dataLengthCond.wait(&_dataLengthMutex);
        qDebug() << "flush()" << "waken up" << dataLength();
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
    QMutexLocker locker(&_dataLengthMutex);

    return _abort || (_size != -1 && readPos() >= totalSize());
}

qint64 FtpBuffer::totalSize() const
{
    QMutexLocker locker(&_mutex);

    return (_size == -1 ? (_basePos + _buffer.pos()) : _size);
}
