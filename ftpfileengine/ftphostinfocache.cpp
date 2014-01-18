#include "ftphostinfocache.h"

QMap<QString, QStringList> FtpHostInfoCache::_hostMap;

FtpHostInfoCache::FtpHostInfoCache(QObject *parent) :
    QObject(parent)
{
}

FtpHostInfoCache::~FtpHostInfoCache()
{

}

QStringList FtpHostInfoCache::hostInfo(const QString& host)
{
    if (!_hostMap.contains(host))
        addHostInfo(host, "anonymous", "anonymous", 21);

    return _hostMap.value(host);
}

void FtpHostInfoCache::addHostInfo(const QString& host,
                                   const QString& userName,
                                   const QString& password,
                                   const QString& port)
{
    _hostMap.insert(host, QStringList() << userName << password << port);
}

void FtpHostInfoCache::addHostInfo(const QString& host,
                                   const QString& userName,
                                   const QString& password, int port)
{
    addHostInfo(host, userName, password, QString::number(port));
}
