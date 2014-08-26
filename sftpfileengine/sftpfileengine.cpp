/****************************************************************************
**
** SFtpFileEngine, class for SFTP access
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

#include "sftpfileengine.h"

#ifdef Q_OS_OS2
#include <types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define closesocket ::close
#elif defined(Q_OS_WIN32)
#include <winsock2.h>
#endif

#include <libssh2.h>
#include <libssh2_sftp.h>

#include <QtCore>

#include "../ftpfileengine/ftpfileengineiterator.h"
#include "../ftpfileengine/ftphostinfocache.h"
#include "../ftpfileengine/ftpfileinfocache.h"
#include "sftpconnectioncache.h"
#include "pathcomp.h"

static
void attr2urlinfo(QUrlInfo *urlInfo, const QString &name,
                  LIBSSH2_SFTP_ATTRIBUTES *sftp_attr)
{
    urlInfo->setName(name);

    urlInfo->setSymLink(LIBSSH2_SFTP_S_ISLNK(sftp_attr->permissions));
    urlInfo->setFile(LIBSSH2_SFTP_S_ISREG(sftp_attr->permissions));
    urlInfo->setDir(LIBSSH2_SFTP_S_ISDIR(sftp_attr->permissions));

    QDateTime dt(QDateTime::fromString("1970-01-01 00:00:00",
                                       "yyyy-MM-dd HH:mm:ss"));
    urlInfo->setLastModified(dt.addSecs(sftp_attr->mtime));
    urlInfo->setLastRead(dt.addSecs(sftp_attr->atime));

    urlInfo->setSize(sftp_attr->filesize);

    int p = 0;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IRUSR)
        p |= QFile::ReadUser | QFile::ReadOwner;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IWUSR)
        p |= QFile::WriteUser | QFile::WriteOwner;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IXUSR)
        p |= QFile::ExeUser | QFile::ExeOwner;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IRGRP)
        p |= QFile::ReadGroup;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IWGRP)
        p |= QFile::WriteGroup;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IXGRP)
        p |= QFile::ExeGroup;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IROTH)
        p |= QFile::ReadOther;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IWOTH)
        p |= QFile::WriteOther;

    if (sftp_attr->permissions & LIBSSH2_SFTP_S_IXOTH)
        p |= QFile::ExeOther;

    urlInfo->setPermissions(p);
    urlInfo->setReadable(p & (LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IROTH));
    urlInfo->setWritable(p & (LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IWOTH));

    // TODO
    // urlInfo->setGroup();
    // urlInfo->setOwner();
}

static inline
QUrlInfo attr2urlinfo(const QString &name, LIBSSH2_SFTP_ATTRIBUTES *sftp_attr)
{
    QUrlInfo urlInfo;

    attr2urlinfo(&urlInfo, name, sftp_attr);

    return urlInfo;
}

SFtpFileEngine::SFtpFileEngine(QObject* parent)
    : QObject(parent)
    , QAbstractFileEngine()
    , _fileFlags(0)
    , _sock(-1)
    , _session(0)
    , _sftp_session(0)
    , _sftp_handle(0)
    , _fileInfoCache(0)
{
    qDebug() << "SFtpFileEngine()";

    initSFtp();
}

SFtpFileEngine::SFtpFileEngine(const QString &fileName, QObject *parent)
    : QObject(parent)
    , QAbstractFileEngine()
    , _fileFlags(0)
    , _sock(-1)
    , _session(0)
    , _sftp_session(0)
    , _sftp_handle(0)
    , _fileInfoCache(0)
{
    qDebug() << "SFtpFileEngine(fileName) : " << fileName;

    initFromFileName(fileName);

    initSFtp();
}

SFtpFileEngine::~SFtpFileEngine()
{
    qDebug() << "~SFtpFileEngine()" << _fileName;
}

void SFtpFileEngine::initFromFileName(const QString& file)
{
    _fileName = PathComp::fixUrl(file);

    _url.setUrl(_fileName);

    FtpHostInfoCache hostInfoCache;

    _userName = _url.userName();
    _password = _url.password();
    if (_password.isEmpty())
        _password = hostInfoCache.password(_url.host(), _userName);

    _port = _url.port(22);

    _encoding = _url.queryItemValue("encoding");
    if (_encoding.isEmpty())
        _encoding = hostInfoCache.encoding(_url.host(), _userName);

    _textCodec = QTextCodec::codecForName(_encoding.toLocal8Bit());

    _url.setUserName(_userName);
    _url.setPassword(_password);
    _url.setPort(_port);

    _url.setQueryItems(QList<QPair<QString, QString> >()
                       << QPair<QString, QString>
                             ("encoding", _encoding));

    hostInfoCache.addHostInfo(_url.host(), _userName, _password, _port,
                              "", _encoding);

    _path = _url.path();
    if (_path.isEmpty())
        _path = "/";

    while (_path.length() > 1 && _path.endsWith("/"))
        _path.chop(1);

    _fileName = _url.scheme().append("://").append(_url.host()).append(_path);
}

void SFtpFileEngine::initSFtp()
{
    _fileInfoCache = FtpFileInfoCache::getInstance();

    QString cacheEntry(getCachePath(_path));

    QUrlInfo urlInfo = _fileInfoCache->findFileInfo(cacheEntry);
    if (urlInfo.isValid())
    {
        // non-existent file ?
        if (urlInfo.permissions() == 0)
        {
            _fileFlags = QAbstractFileEngine::FileType;

            return;
        }

        _fileFlags = QAbstractFileEngine::ExistsFlag;

        _fileFlags |= urlInfo.isDir() ? QAbstractFileEngine::DirectoryType :
                                        QAbstractFileEngine::FileType;

        if (_path == "/")
            _fileFlags |= QAbstractFileEngine::RootFlag;

        _fileFlags |= QAbstractFileEngine::FileFlag(urlInfo.permissions());

        _urlInfo = urlInfo;

        return;
    }

    refreshFileInfoCache();
}

void SFtpFileEngine::readDir(const QString &dir)
{
    QString cacheDir(getCachePath(dir, true));

    _fileInfoCache->removeDirInfo(cacheDir);

    // add an empty entry to distinguish a empty directory from
    // a non-existent directory
    _fileInfoCache->addFileInfo(cacheDir, QUrlInfo());

    LIBSSH2_SFTP_HANDLE* dir_handle;

    dir_handle = libssh2_sftp_opendir(_sftp_session,
                                      _textCodec->fromUnicode(dir).data());
    if (dir_handle)
    {
        QByteArray entry(512, 0);
        QByteArray longEntry(512, 0);
        LIBSSH2_SFTP_ATTRIBUTES attr;

        while (libssh2_sftp_readdir_ex(dir_handle, entry.data(),
                                       entry.capacity(), longEntry.data(),
                                       longEntry.capacity(), &attr) > 0)
        {
            QString entryUni(_textCodec->toUnicode(entry.data()));

            QUrlInfo urlInfo;
            attr2urlinfo(&urlInfo, entryUni, &attr);

            _fileInfoCache->addFileInfo(cacheDir, urlInfo);
        }

        libssh2_sftp_closedir(dir_handle);
    }
}

void SFtpFileEngine::refreshFileInfoCache()
{
    // sftp: case ?
    if (_url.host().isEmpty())
    {
        qDebug() << "refreshFileInfoCache()" << "host empty" << _path;

        // accepts / only
        if (_path != "/")
        {
            _fileFlags = QAbstractFileEngine::FileType;

            return;
        }

        QAbstractFileEngine::FileFlags permissions =
                QAbstractFileEngine::ReadOwnerPerm |
                QAbstractFileEngine::ReadUserPerm |
                QAbstractFileEngine::ReadGroupPerm |
                QAbstractFileEngine::ReadOtherPerm;

        _fileFlags = QAbstractFileEngine::RootFlag |
                QAbstractFileEngine::ExistsFlag |
                QAbstractFileEngine::DirectoryType |
                permissions;

        _urlInfo.setName(_path);
        _urlInfo.setDir(true);
        _urlInfo.setPermissions(permissions);

        QString cacheDir(getCachePath(_path, true));

        _fileInfoCache->addFileInfo(cacheDir, _urlInfo);

        // insert a invalid entry to avoid cache failure in beginEntryList()
        _fileInfoCache->addFileInfo(cacheDir, QUrlInfo());

        return;
    }

    // failed to connect or to login ?
    if (!sftpConnect())
    {
        // Do not cache.
        // It is possbiel that a server was down and it will be up later.
        // and caching a wrong account is useless

        _fileFlags = QAbstractFileEngine::FileType;

        return;
    }

    if (_path == "/")
    {
        QAbstractFileEngine::FileFlags permissions =
                QAbstractFileEngine::ReadOwnerPerm |
                QAbstractFileEngine::ReadUserPerm |
                QAbstractFileEngine::ReadGroupPerm |
                QAbstractFileEngine::ReadOtherPerm;

        _fileFlags = QAbstractFileEngine::RootFlag |
                     QAbstractFileEngine::ExistsFlag |
                     QAbstractFileEngine::DirectoryType |
                     permissions;

        _urlInfo.setName(_path);
        _urlInfo.setDir(true);
        _urlInfo.setPermissions(permissions);

        _fileInfoCache->addFileInfo(getCachePath(_path, true), _urlInfo);
    }
    else
    {
        QString dir(PathComp(_path).dir());
        QString name(PathComp(_path).fileName());

        readDir(dir);

        _urlInfo = _fileInfoCache->findFileInfo(getCachePath(_path));

        _fileFlags = 0;

        if (_urlInfo.isValid())
        {
            _fileFlags = QAbstractFileEngine::ExistsFlag;

            _fileFlags |= _urlInfo.isDir() ?
                                QAbstractFileEngine::DirectoryType :
                                QAbstractFileEngine::FileType;

            if (_path == "/")
                _fileFlags |= QAbstractFileEngine::RootFlag;

            _fileFlags |=
                    QAbstractFileEngine::FileFlag(_urlInfo.permissions());
        }
        else if (name != ":refresh:")   // do not cache a refresh signal
        {
            // add a non-existent entry as well not to read a directory
            // to retrive its information and to test its existence again
            // later
            _fileFlags |= QAbstractFileEngine::FileType;

            _urlInfo.setName(name);
            _urlInfo.setPermissions(0);

            _fileInfoCache->addFileInfo(getCachePath(dir, true), _urlInfo);
        }
    }

    sftpDisconnect();
}

void SFtpFileEngine::refreshFileInfoCache(const QString &path)
{
    if (_path == path)
    {
        refreshFileInfoCache();

        return;
    }

    // failed to connect or to login ?
    if (!sftpConnect())
        return;

    readDir(PathComp(path).dir());

    sftpDisconnect();
}

bool SFtpFileEngine::sftpConnect()
{
    SFtpConnectionCache* cache = SFtpConnectionCache::getInstance();

    const SFtpConnectionCache::SFtpConnection *conn =
            cache->findConnection(_url);

    if (conn)
    {
        _sock = conn->sock;
        _session = conn->session;
        _sftp_session = conn->sftp_session;

        return true;
    }

    struct hostent *host;

    host = gethostbyname(_url.host().toLocal8Bit().constData());
    if (!host)
        return false;

    _sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(_port);
    sin.sin_addr.s_addr = *reinterpret_cast<u_long *>(host->h_addr);
    if (::connect(_sock, reinterpret_cast<struct sockaddr *>(&sin),
                  sizeof(struct sockaddr_in)) != 0)
    {
        closesocket(_sock);

        return false;
    }

    _session = libssh2_session_init();
    if(!_session)
    {
        closesocket(_sock);

        return false;
    }

    libssh2_session_set_blocking(_session, 1);

    if (libssh2_session_handshake(_session, _sock))
    {
        libssh2_session_free(_session);
        closesocket(_sock);

        return false;
    }

    if (libssh2_userauth_password(_session,
                                  _textCodec->fromUnicode(_userName).data(),
                                  _textCodec->fromUnicode(_password).data()))
    {
        libssh2_session_disconnect(_session, "Abnormal Shutdown");
        libssh2_session_free(_session);
        closesocket(_sock);

        return false;
    }

    _sftp_session = libssh2_sftp_init(_session);
    if (!_sftp_session)
    {
        libssh2_session_disconnect(_session, "Abnormal Shutdown");
        libssh2_session_free(_session);
        closesocket(_sock);


        return false;
    }

    libssh2_session_set_blocking(_session, 1);
    libssh2_session_set_timeout(_session, 30 * 1000);

    cache->addConnection(_url, _sock, _session, _sftp_session);

    return true;
}

bool SFtpFileEngine::sftpDisconnect()
{
    _sftp_session = 0;
    _session = 0;
    _sock = -1;

    return true;
}

bool SFtpFileEngine::atEnd() const
{
    qDebug() << "atEnd()" << _fileName;

    return _urlInfo.size() >= libssh2_sftp_tell64(_sftp_handle);
}

static QStringList filterEntries(QDir::Filters filters,
                                 const QStringList& filterNames,
                                 const QMap<QString, QUrlInfo>& entriesMap)
{
    QStringList entries;

    QMapIterator<QString, QUrlInfo> it(entriesMap);

    while (it.hasNext())
    {
        it.next();

        if ((filters & QDir::AllDirs) && it.value().isDir())
        {
            if ((filters & (QDir::NoDot | QDir::NoDotAndDotDot))
                    && it.value().name() == ".")
                continue;

            if ((filters & (QDir::NoDotDot | QDir::NoDotAndDotDot))
                    && it.value().name() == "..")
                continue;

            entries.append(it.key());

            continue;
        }

        bool matched = false;

        if ((filters & QDir::Dirs) && it.value().isDir())
        {
            if ((filters & (QDir::NoDot | QDir::NoDotAndDotDot))
                    && it.value().name() == ".")
                continue;

            if ((filters & (QDir::NoDotDot | QDir::NoDotAndDotDot))
                    && it.value().name() == "..")
                continue;

            matched = true;
        }

        matched = matched ||
                  ((filters & QDir::Files) && it.value().isFile()) ||
                  ((filters & QDir::Readable) && it.value().isReadable()) ||
                  ((filters & QDir::Writable) && it.value().isWritable()) ||
                  ((filters & QDir::Executable)
                        && it.value().isExecutable()) ||
                  ((filters & QDir::Hidden)
                        && it.value().name().startsWith("."));

        if (matched  && (filterNames.isEmpty()
                         || QDir::match(filterNames, it.value().name())))
                entries.append(it.key());
    }

    return entries;
}

QAbstractFileEngine::Iterator*
SFtpFileEngine::beginEntryList(QDir::Filters filters,
                              const QStringList &filterNames)
{
    qDebug() << "beginEntryList() : " << _fileName << _fileFlags;

    QString cachePath(getCachePath(_path, true));
    FtpFileInfoCache::QUrlInfoList list =
            _fileInfoCache->findDirInfo(cachePath);

    if (list.size() == 0
            && (_fileFlags & QAbstractFileEngine::DirectoryType)
            && sftpConnect())
    {
        readDir(_path);

        sftpDisconnect();
    }

    QMap<QString, QUrlInfo> entriesMap;

    list = _fileInfoCache->findDirInfo(cachePath);
    if (list.size() > 0)
    {
        FtpFileInfoCache::QUrlInfoListIterator it(list);
        while (it.hasNext())
        {
            QUrlInfo urlInfo = it.next();

            // exclude an empty entry inserted by us and a non-existent entry
            if (urlInfo.isValid() && urlInfo.permissions())
                entriesMap.insert(urlInfo.name(), urlInfo);
        }
    }

    return new FtpFileEngineIterator(filters, filterNames,
                                     filterEntries(filters, filterNames,
                                                   entriesMap));
}

bool SFtpFileEngine::caseSensitive() const
{
    qDebug() << "caseSensitive()" << _fileName;

    return true;
}

bool SFtpFileEngine::close()
{
    qDebug() << "close()" << _fileName;

    libssh2_sftp_close(_sftp_handle);

    _sftp_handle = 0;

    sftpDisconnect();

    return true;
}

bool SFtpFileEngine::copy(const QString &newName)
{
    Q_UNUSED(newName);

    qDebug() << "copy()" << _fileName;

    return false;
}

QStringList SFtpFileEngine::entryList(QDir::Filters filters,
                                     const QStringList &filterNames) const
{
    qDebug() << "entryList()" << _fileName;

    return QAbstractFileEngine::entryList(filters, filterNames);
}

QFile::FileError SFtpFileEngine::error() const
{
    qDebug() << "error()" << _fileName;

    return QAbstractFileEngine::error();
}

QString SFtpFileEngine::errorString() const
{
    qDebug() << "errorString()" << _fileName;

    return QAbstractFileEngine::errorString();
}

bool SFtpFileEngine::extension(Extension extension,
                              const ExtensionOption *option,
                              ExtensionReturn *output)
{
    qDebug() << "extension()" << _fileName;

    return QAbstractFileEngine::extension(extension, option, output);
}

QAbstractFileEngine::FileFlags SFtpFileEngine::fileFlags(FileFlags type) const
{
    qDebug() << "fileFlags() : " << _fileName << type << _fileFlags;

    qDebug() << "\t" << _fileName << (type & _fileFlags);

    return type & _fileFlags;
}

QString SFtpFileEngine::fileName(FileName file) const
{
    qDebug() << "fileName()" << _fileName << file;

    QString result(_url.scheme());

    result.append("://");

    if (!_userName.isEmpty())
    {
        result.append(_userName);

        if (file == QAbstractFileEngine::CanonicalName
                || file == QAbstractFileEngine::CanonicalPathName)
        {
            if (!_password.isEmpty())
            {
                result.append(":");
                result.append(_password);
            }
        }

        result.append("@");
    }

    result.append(_url.host());

    if (_port != 22)
    {
        result.append(":");
        result.append(QString::number(_port));
    }

    switch (file)
    {
    case QAbstractFileEngine::DefaultName:
        result.append(_path);
        break;

    case QAbstractFileEngine::BaseName:
        result = PathComp(_path).fileName();
        break;

    case QAbstractFileEngine::PathName:
        result.append(PathComp(_path).dir());
        break;

    case QAbstractFileEngine::AbsoluteName:
    case QAbstractFileEngine::CanonicalName:
        result.append(_path);
        break;

    case QAbstractFileEngine::AbsolutePathName:
    case QAbstractFileEngine::CanonicalPathName:
        result.append(PathComp(_path).dir());
        break;

    case QAbstractFileEngine::LinkName:
    case QAbstractFileEngine::BundleName:
    default:
        break;
    }

    if (file == QAbstractFileEngine::CanonicalName
            || file == QAbstractFileEngine::CanonicalPathName)
    {
        result.append("?encoding=").append(_encoding);
    }

    qDebug() << "\t" << result;

    return result;
}

QDateTime SFtpFileEngine::fileTime(FileTime time) const
{
    qDebug() << "fileTime() " << _fileName << time;

    switch (time)
    {
    case QAbstractFileEngine::ModificationTime:
        return _urlInfo.lastModified();

    case QAbstractFileEngine::AccessTime:
        return _urlInfo.lastRead();

    default:
        break;
    }

    return QDateTime();
}

bool SFtpFileEngine::flush()
{
    qDebug() << "flush()" << _fileName;

    return true;
}

int SFtpFileEngine::handle() const
{
    qDebug() << "handle()" << _fileName;

    return QAbstractFileEngine::handle();
}

bool SFtpFileEngine::isRelativePath() const
{
    qDebug() << "isRelativePath()" << _fileName;

    return _url.isRelative();
}

bool SFtpFileEngine::isSequential() const
{
    qDebug() << "isSequential()" << _fileName;

    return true;
}

bool SFtpFileEngine::link(const QString &newName)
{
    Q_UNUSED(newName);

    qDebug() << "link()" << _fileName;

    return false;
}

uchar* SFtpFileEngine::map(qint64 offset, qint64 size,
                          QFile::MemoryMapFlags flags)
{
    qDebug() << "map()" << _fileName;

    return QAbstractFileEngine::map(offset, size, flags);
}

bool SFtpFileEngine::mkdir(const QString &dirName,
                           bool createParentDirectories) const
{
    Q_UNUSED(createParentDirectories);

    qDebug() << "mkdir()" << _fileName << dirName;

    SFtpFileEngine* This(const_cast<SFtpFileEngine *>(this));
    if (!This->sftpConnect())
        return false;

    QUrl url(PathComp::fixUrl(dirName));

    bool result = !libssh2_sftp_mkdir(_sftp_session,
                                      _textCodec->fromUnicode(
                                          url.path()).data(),
                                      LIBSSH2_SFTP_S_IRWXU |
                                      LIBSSH2_SFTP_S_IRGRP |
                                      LIBSSH2_SFTP_S_IXGRP |
                                      LIBSSH2_SFTP_S_IROTH |
                                      LIBSSH2_SFTP_S_IXOTH);

    This->sftpDisconnect();

    if (result)
        This->refreshFileInfoCache(url.path());

    return result;
}

bool SFtpFileEngine::open(QIODevice::OpenMode openMode)
{
    qDebug() << "open()" << _fileName << openMode;

    if (_fileFlags & QAbstractFileEngine::DirectoryType)
        return false;

    if (!(openMode & (QIODevice::ReadOnly | QIODevice::WriteOnly)))
        return false;

    if ((openMode & QIODevice::ReadOnly)
            && (!(_fileFlags & QAbstractFileEngine::ExistsFlag)))
        return false;

    if (!sftpConnect())
        return false;

    int flags = 0;
    int mode = 0;

    if (openMode & QIODevice::ReadOnly)
        flags |= LIBSSH2_FXF_READ;

    if (openMode & QIODevice::WriteOnly)
    {
        flags |= LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT;
        mode |= LIBSSH2_SFTP_S_IRUSR |
                LIBSSH2_SFTP_S_IWUSR |
                LIBSSH2_SFTP_S_IRGRP |
                LIBSSH2_SFTP_S_IROTH;
    }

    _sftp_handle = libssh2_sftp_open(_sftp_session,
                                     _textCodec->fromUnicode(_path).data(),
                                     flags, mode);

    return _sftp_handle;
}

QString SFtpFileEngine::owner(FileOwner owner) const
{
    qDebug() << "owner()" << _fileName << owner;
    qDebug() << "\t" << _urlInfo.owner();

    return QAbstractFileEngine::owner(owner);
}

uint SFtpFileEngine::ownerId(FileOwner owner) const
{
    qDebug() << "ownerId() : " << _fileName << owner;

    return QAbstractFileEngine::ownerId(owner);
}

qint64 SFtpFileEngine::pos() const
{
    qDebug() << "pos()" << _fileName;

    return 0;
}

qint64 SFtpFileEngine::read(char *data, qint64 maxlen)
{
    qDebug() << "read()" << _fileName;

    return libssh2_sftp_read(_sftp_handle, data, maxlen);
}

qint64 SFtpFileEngine::readLine(char *data, qint64 maxlen)
{
    qDebug() << "readLine()" << _fileName;

    return QAbstractFileEngine::readLine(data, maxlen);
}

bool SFtpFileEngine::remove()
{
    qDebug() << "remove()" << _fileName;

    if (!sftpConnect())
        return false;

    bool result = !libssh2_sftp_unlink(_sftp_session,
                                       _textCodec->fromUnicode(_path).data());

    if (result)
    {
        // remove cache entry
        _fileInfoCache->removeFileInfo(getCachePath(_path));
        _fileFlags = QAbstractFileEngine::FileType;
    }

    sftpDisconnect();

    return result;
}

bool SFtpFileEngine::rename(const QString &newName)
{
    qDebug() << "rename()" << _fileName << _path << newName;

    if (!sftpConnect())
        return false;

    PathComp fixedNewName(newName);
    QString newPath(QUrl(fixedNewName.path()).path());

    bool result = !libssh2_sftp_rename(_sftp_session,
                                       _textCodec->fromUnicode(_path).data(),
                                       _textCodec->fromUnicode(newPath).data());

    if (result)
    {
        _fileInfoCache->renameFileInfo(getCachePath(_path),
                                       getCachePath(newPath));
        _urlInfo.setName(fixedNewName.fileName());
    }

    sftpDisconnect();

    return result;
}

bool SFtpFileEngine::rmdir(const QString &dirName,
                          bool recurseParentDirectories) const
{
    Q_UNUSED(recurseParentDirectories);

    qDebug() << "rmdir()" << _fileName << dirName;

    SFtpFileEngine* This(const_cast<SFtpFileEngine*>(this));

    if (!This->sftpConnect())
        return false;

    QUrl url(PathComp::fixUrl(dirName));

    bool result = !libssh2_sftp_rmdir(_sftp_session,
                                      _textCodec->fromUnicode(
                                          url.path()).data());

    if (result)
    {
        // remove cache entry
        This->_fileInfoCache->removeFileInfo(This->getCachePath(url.path()));
        if (_path == url.path())
            This->_fileFlags = QAbstractFileEngine::FileType;
    }

    This->sftpDisconnect();

    return result;
}

bool SFtpFileEngine::seek(qint64 pos)
{
    Q_UNUSED(pos);

    qDebug() << "seek()" << _fileName;

    return false;
}

void SFtpFileEngine::setFileName(const QString &file)
{
    qDebug() << "setFileName() : " << file;

    initFromFileName(file);
}

bool SFtpFileEngine::setPermissions(uint perms)
{
    qDebug() << "setPermission()" << _fileName;

    return QAbstractFileEngine::setPermissions(perms);
}

bool SFtpFileEngine::setSize(qint64 size)
{
    qDebug() << "setSize()" << _fileName << size;

    return false;
}

qint64 SFtpFileEngine::size() const
{
    qDebug() << "size()" << _fileName;
    qDebug() << "\t" << _urlInfo.size();

    return _urlInfo.size();
}

bool SFtpFileEngine::supportsExtension(Extension extension) const
{
    qDebug() << "supportsExtension()" << _fileName;

    return QAbstractFileEngine::supportsExtension(extension);
}

bool SFtpFileEngine::unmap(uchar *ptr)
{
    qDebug() << "unmap()" << _fileName;

    return QAbstractFileEngine::unmap(ptr);
}

qint64 SFtpFileEngine::write(const char *data, qint64 len)
{
    qDebug() << "write()" << _fileName << len;

    return libssh2_sftp_write(_sftp_handle, data, len);
}

QString SFtpFileEngine::getCachePath(const QString &path, bool key) const
{
    QString cachePath(_url.scheme());

    cachePath.append("://").append(_userName).append("@")
             .append(_url.host()).append(":").append(QString::number(_port))
             .append(path);

    if (path.isEmpty())
        cachePath.append("/");

    if (key && !cachePath.endsWith("/"))
        cachePath.append("/");

    return cachePath;
}
