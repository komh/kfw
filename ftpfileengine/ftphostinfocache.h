#ifndef FTPHOSTINFOCACHE_H
#define FTPHOSTINFOCACHE_H

#include <QMap>
#include <QString>
#include <QStringList>

#include <QObject>

class FtpHostInfoCache : public QObject
{
    Q_OBJECT
public:
    explicit FtpHostInfoCache(QObject *parent = 0);
    ~FtpHostInfoCache();

    enum { UserName = 0, Password = 1, Port = 2 };

    QStringList hostInfo(const QString& host);

    QString userName(const QString& host)
    {
        return hostInfo(host).at(UserName);
    }

    QString password(const QString& host)
    {
        return hostInfo(host).at(Password);
    }

    int port(const QString& host)
    {
        return hostInfo(host).at(Port).toInt();
    }

    void addHostInfo(const QString& host, const QString& userName,
                     const QString& password, const QString& port);

    void addHostInfo(const QString& host, const QString& userName,
                     const QString& password, int port);

private:
    static QMap<QString, QStringList> _hostMap;
};

#endif // FTPHOSTINFOCACHE_H
