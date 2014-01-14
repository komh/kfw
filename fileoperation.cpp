#include "fileoperation.h"

FileOperation::FileOperation(const QString &source, const QString &dest)
    : _source(source)
    , _dest(dest)
{
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
