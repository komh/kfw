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

    return _destFile.open(QIODevice::WriteOnly);
}

void FileOperation::close()
{
    _sourceFile.close();
    _destFile.close();
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
