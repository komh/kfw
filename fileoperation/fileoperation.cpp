/****************************************************************************
**
** FileOperations, class for file operations
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

#include <QFile>

#include "fileoperation.h"

FileOperation::FileOperation(const QString &source, const QString &dest,
                             QObject *parent)
    : QObject(parent)
    , _source(source)
    , _dest(dest)
{
}

bool FileOperation::open()
{
    close();

    if (source().isEmpty())
        return false;

    _sourceFile.setFileName(source());

    if (!_sourceFile.open(QIODevice::ReadOnly))
        return false;

    if (dest().isEmpty())
        return true;

    _destFile.setFileName(dest());

    // Set the size of a file on FTP to help it to determine EOF
    if (dest().startsWith("ftp://"))
        _destFile.resize(_sourceFile.size());

    return _destFile.open(QIODevice::WriteOnly);
}

void FileOperation::close()
{
    _sourceFile.close();
    _destFile.close();
}

void FileOperation::abort()
{
    // abort FTP transfer
    if (source().startsWith("ftp://"))
        _sourceFile.resize(-1);

    if (dest().startsWith("ftp://"))
        _destFile.resize(-1);
}

qint64 FileOperation::size() const
{
    return _sourceFile.isOpen() ? _sourceFile.size() : -1;
}

qint64 FileOperation::copy(qint64 chunkSize)
{
    if (!_sourceFile.isOpen() || !_destFile.isOpen())
        return -1;

    QByteArray sourceData(chunkSize, 0);

    int len = _sourceFile.read(sourceData.data(), chunkSize);

    // -1 for error, 0 for nothing read
    if (len <= 0)
        return len;

    if (_destFile.write(sourceData.data(), len) != len)
        return -1;

    return len;
}

bool FileOperation::remove()
{
    return _sourceFile.remove();
}

bool FileOperation::rename(const QString &newName)
{
    if ( _sourceFile.rename(newName))
    {
        setSource(newName);

        return true;
    }

    return false;
}

QString FileOperation::fixUrl(const QString &url)
{
    static int colonSlashLength = QString(":/").length();

    // QFileSystemModel does not recognize URL correctly.
    // Always use xxx:/yyy style for a URL as well as a local path
    QString fixedUrl(url);
    int index = fixedUrl.indexOf(":/");
    if (index > 1 && fixedUrl.at(index + colonSlashLength) != '/')
        fixedUrl.replace(index, colonSlashLength, "://");

    return fixedUrl;
}
