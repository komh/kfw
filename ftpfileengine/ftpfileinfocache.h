#ifndef FTPFILEINFOCACHE_H
#define FTPFILEINFOCACHE_H

#include <QtCore>

#include <QUrlInfo>

class FtpFileInfoCache
{
public:
    void addFileInfo(const QString& dir, const QUrlInfo& urlInfo);
    void removeDirInfo(const QString& dir);

    QUrlInfo findFileInfo(const QString& dir, const QString& name);
    QUrlInfo findFileInfo(const QString& path);

    typedef QList<QUrlInfo> QUrlInfoList;
    typedef QListIterator<QUrlInfo> QUrlInfoListIterator;

    QUrlInfoList findDirInfo(const QString& dir);

    static FtpFileInfoCache* getInstance();

private:
    explicit FtpFileInfoCache();
    ~FtpFileInfoCache();

    static QMultiMap<QString, QUrlInfo> _dirMultiMap;

    QString getCacheKey(const QString& dir, const QString &name = QString());
};

#endif // FTPFILEINFOCACHE_H
