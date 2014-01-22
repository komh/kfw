#ifndef FTPFILEENGINE_H
#define FTPFILEENGINE_H

#include <QtCore>
#include <QFtp>

#include "ftpsync.h"
#include "ftpfileinfocache.h"
#include "ftpbuffer.h"

class FtpTransferThread;

class FtpFileEngine : public QObject, public QAbstractFileEngine
{
    Q_OBJECT

public:
    explicit FtpFileEngine(QObject* parent = 0);
    explicit FtpFileEngine(const QString& fileName, QObject* parent = 0);
    ~FtpFileEngine();

    bool atEnd() const;
    Iterator* beginEntryList(QDir::Filters filters,
                             const QStringList &filterNames);
    bool caseSensitive() const;
    bool close();
    bool copy(const QString &newName);
    QStringList entryList(QDir::Filters filters,
                          const QStringList &filterNames) const;
    QFile::FileError error() const;
    QString errorString() const;
    bool extension(Extension extension, const ExtensionOption *option,
                   ExtensionReturn *output);
    FileFlags fileFlags(FileFlags type) const;
    QString fileName(FileName file) const;
    QDateTime fileTime(FileTime time) const;
    bool flush();
    int handle() const;
    bool isRelativePath() const;
    bool isSequential() const;
    bool link(const QString &newName);
    uchar* map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
    bool mkdir(const QString &dirName, bool createParentDirectories) const;
    bool open(QIODevice::OpenMode openMode);
    QString owner(FileOwner owner) const;
    uint ownerId(FileOwner owner) const;
    qint64 pos() const;
    qint64 read(char *data, qint64 maxlen);
    qint64 readLine(char *data, qint64 maxlen);
    bool remove();
    bool rename(const QString &newName);
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const;
    bool seek(qint64 pos);
    void setFileName(const QString &file);
    bool setPermissions(uint perms);
    bool setSize(qint64 size);
    qint64 size() const;
    bool supportsExtension(Extension extension) const;
    bool unmap(uchar *ptr);
    qint64 write(const char *data, qint64 len);

private slots:
    void ftpListInfo(const QUrlInfo& urlInfo);

private:
    friend class FtpTransferThread;

    QFtp* _ftp;

    QString _fileName;
    QAbstractFileEngine::FileFlags _fileFlags;
    QUrlInfo _urlInfo;
    FtpBuffer _fileBuffer;

    QUrl _url;
    QString _userName;
    QString _password;
    int _port;
    QString _path;

    QString _cacheDir;

    QStringList _entries;
    QMap<QString, QUrlInfo> _entriesMap;

    FtpSync _ftpSync;

    FtpFileInfoCache* _ftpCache;

    FtpTransferThread* _ftpTransfer;

    void initFromFileName(const QString& file = QString());
    void initFtp();
    void refreshFileInfoCache();
    QString getCachePath(const QString& path, bool key = false);

private slots:
    void abortBufferAccess();
};
#endif // FTPFILEENGINE_H
