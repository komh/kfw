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

    void abort();
    bool  flush();
    qint64 readPos() const;
    void setSize(qint64 size) { _size = size; _dataLengthCond.wakeAll();}

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    QBuffer _buffer;
    qint64 _basePos;
    qint64 _readPos;
    mutable QMutex _mutex;
    mutable QMutex _dataLengthMutex;
    mutable QWaitCondition _dataLengthCond;

    bool _abort;
    qint64 _size;

    qint64 dataLength() const;
    bool isEnd() const;
    qint64 totalSize() const;
};

#endif // FTPBUFFER_H
