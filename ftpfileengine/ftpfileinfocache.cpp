#include "ftpfileinfocache.h"

QMultiMap<QString, QUrlInfo> FtpFileInfoCache::_dirMultiMap;

FtpFileInfoCache::FtpFileInfoCache()
{
}

FtpFileInfoCache::~FtpFileInfoCache()
{
}

void FtpFileInfoCache::addFileInfo(const QString& dir, const QUrlInfo& urlInfo)
{
    _dirMultiMap.insert(getCacheKey(dir, urlInfo.name()), urlInfo);
}

QUrlInfo FtpFileInfoCache::findFileInfo(const QString& dir,
                                        const QString& name)
{
    QString cacheKey = getCacheKey(dir, name);

    if (_dirMultiMap.contains(cacheKey))
    {
        QUrlInfoListIterator it(_dirMultiMap.values(cacheKey));

        while (it.hasNext())
        {
            QUrlInfo info = it.next();

            if (name == info.name())
                return info;
        }
    }

    return QUrlInfo();
}

QUrlInfo FtpFileInfoCache::findFileInfo(const QString& path)
{
    int lastSlashIndex = path.lastIndexOf("/");
    QString dir(path.left(lastSlashIndex));
    QString name(path.mid(lastSlashIndex + 1));

    // Treat root like a file entry
    if (name.isEmpty())
        name = "/";

    return findFileInfo(dir, name);
}

FtpFileInfoCache::QUrlInfoList
FtpFileInfoCache::findDirInfo(const QString& dir)
{
    QString cacheKey = getCacheKey(dir, "");

    if (_dirMultiMap.contains(cacheKey))
        return _dirMultiMap.values(cacheKey);

    return QUrlInfoList();
}

FtpFileInfoCache* FtpFileInfoCache::getInstance()
{
    static FtpFileInfoCache cache;

    return &cache;
}

QString FtpFileInfoCache::getCacheKey(const QString& dir, const QString & name)
{
    QString cacheKey(dir);

    if (!cacheKey.endsWith("/"))
        cacheKey.append("/");

    // Use "ftp://a.b.c.d" as a cach key for "ftp://a.b.c.d/"
    // to distinguish from the cache key of "ftp://a.b.c.d/xxx"
    if (name == "/")
        cacheKey.chop(1);

    return cacheKey;
}
