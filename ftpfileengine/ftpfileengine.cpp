/****************************************************************************
**
** FtpFileEngine, class for FTP access
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

#include <QtCore>

#include "ftpsync.h"
#include "ftpfileengineiterator.h"
#include "ftpfileinfocache.h"
#include "fileoperation/fileoperation.h"
#include "ftpbuffer.h"
#include "ftptransferthread.h"
#include "ftphostinfocache.h"
#include "ftpconnectioncache.h"
#include "pathcomp.h"

#include "ftpfileengine.h"

FtpFileEngine::FtpFileEngine(QObject* parent)
    : QObject(parent)
    , QAbstractFileEngine()
    , _ftp(0)
    , _ftpConnected(false)
    , _fileFlags(0)
    , _ftpCache(0)
    , _ftpTransfer(0)
{
    qDebug() << "FtpFileEngine()";

    initFtp();
}

FtpFileEngine::FtpFileEngine(const QString &fileName, QObject *parent)
    : QObject(parent)
    , QAbstractFileEngine()
    , _ftp(0)
    , _ftpConnected(false)
    , _fileFlags(0)
    , _ftpCache(0)
    , _ftpTransfer(0)
{
    qDebug() << "FtpFileEngine(fileName) : " << fileName;

    initFromFileName(fileName);

    initFtp();
}

FtpFileEngine::~FtpFileEngine()
{
    qDebug() << "~FtpFileEngine()" << _fileName;

#if 0
    close();
#endif
}

void FtpFileEngine::initFromFileName(const QString& file)
{
    _fileName = PathComp::fixUrl(file);

    _url.setUrl(_fileName);

    FtpHostInfoCache hostCache;

    _userName = _url.userName();
    if (_userName.isEmpty())
        _userName = "anonymous";

    _password = _url.password();
    if (_password.isEmpty())
    {
        _password = _userName == "anonymous" ?
                    "kfw@kfw.com" : hostCache.password(_url.host(), _userName);
    }

    _port = _url.port(21);

    _transferMode = _url.queryItemValue("transfermode");
    if (_transferMode.isEmpty())
        _transferMode = hostCache.transferMode(_url.host(), _userName);

    _encoding = _url.queryItemValue("encoding");
    if (_encoding.isEmpty())
        _encoding = hostCache.encoding(_url.host(), _userName);

    _textCodec = QTextCodec::codecForName(_encoding.toLatin1());

    _url.setUserName(_userName);
    _url.setPassword(_password);
    _url.setPort(_port);

    _url.setQueryItems(QList<QPair<QString, QString> >()
                       << QPair<QString, QString>
                             ("transfermode", _transferMode)
                       << QPair<QString, QString>
                             ("encoding", _encoding));

    hostCache.addHostInfo(_url.host(), _userName, _password, _port,
                          _transferMode, _encoding);

    _path = _url.path();
    if (_path.isEmpty())
        _path = "/";

    while (_path.length() > 1 && _path.endsWith("/"))
        _path.chop(1);

    _fileName = _url.scheme().append("://").append(_url.host()).append(_path);
}

void FtpFileEngine::initFtp()
{
    qRegisterMetaType<QUrlInfo>("QUrlInfo");

    // close all the connection
    if (_url.host().isEmpty() && _path == "/:closeall:")
    {
        FtpConnectionCache::getInstance()->closeAll();

        _fileFlags = QAbstractFileEngine::FileType;

        return;
    }

    _ftpCache = FtpFileInfoCache::getInstance();

    QString cacheEntry(getCachePath(_path));

    QUrlInfo urlInfo = _ftpCache->findFileInfo(cacheEntry);
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

void FtpFileEngine::readDir(const QString &dir)
{
    _cacheDir = getCachePath(dir, true);

    _ftpCache->removeDirInfo(_cacheDir);

    _entriesMap.clear();

    // get a file list from a parent directory
    _ftp->cd(_textCodec->fromUnicode(dir));
    _ftp->list();

    _ftpSync.wait();
}

void FtpFileEngine::refreshFileInfoCache()
{
    // ftp: case ?
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

        _ftpCache->addFileInfo(cacheDir, _urlInfo);

        // insert a invalid entry to avoid cache failure in beginEntryList()
        _ftpCache->addFileInfo(cacheDir, QUrlInfo());

        return;
    }

    // failed to connect or to login ?
    if (!ftpConnect())
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

        _ftpCache->addFileInfo(getCachePath(_path, true), _urlInfo);
    }
    else
    {
        QString dir(PathComp(_path).dir());
        QString name(PathComp(_path).fileName());

        readDir(dir);

        _urlInfo = _entriesMap.value(name);

        _fileFlags = 0;

        if (_entriesMap.keys().contains(name))
        {
            _fileFlags |= QAbstractFileEngine::ExistsFlag;

            _fileFlags |= _urlInfo.isDir() ?
                            QAbstractFileEngine::DirectoryType :
                            QAbstractFileEngine::FileType;

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

            _ftpCache->addFileInfo(_cacheDir, _urlInfo);
        }
    }

    ftpDisconnect();
}

void FtpFileEngine::refreshFileInfoCache(const QString& path)
{
    if (_path == path)
    {
        refreshFileInfoCache();

        return;
    }

    // failed to connect or to login ?
    if (!ftpConnect())
        return;

    readDir(PathComp(path).dir());

    ftpDisconnect();
}

bool FtpFileEngine::ftpConnect()
{
#ifdef USE_FTP_CONNECTION_CACHE
    FtpConnectionCache* cache = FtpConnectionCache::getInstance();

    QFtp* cachedFtp = cache->findConnection(_url);

    if (cachedFtp)
        _ftp = cachedFtp;
    else
#endif
    _ftp = new QFtp;

    if (!_ftp)
        return false;

    _ftpSync.setFtp(_ftp);

    connect(_ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(ftpListInfo(QUrlInfo)));

#ifdef USE_FTP_CONNECTION_CACHE
    if (_ftp == cachedFtp)
    {
        _ftpConnected = true;

        return true;
    }
#endif

    _ftp->setTransferMode(_transferMode == "Passive" ?
                              QFtp::Passive : QFtp::Active);

    _ftp->connectToHost(_url.host(), _port);

    // wait at most 30s
    _ftpConnected = _ftpSync.wait(30 * 1000);

    if (_ftpSync.timedOut())
        _ftp->abort();

    if (_ftpConnected)
    {
        _ftp->login(_userName, _password);

        if (!_ftpSync.wait())
            ftpDisconnect();
    }

#ifdef USE_FTP_CONNECTION_CACHE
    if (_ftpConnected)
        cache->addConnection(_url, _ftp);
#endif

    return _ftpConnected;
}

bool FtpFileEngine::ftpDisconnect()
{
    if (!_ftpConnected)
        return true;

#ifndef USE_FTP_CONNECTION_CACHE
    _ftp->close();
    _ftpSync.wait();
#endif

    _ftpSync.setFtp(0);

    disconnect(_ftp, SIGNAL(listInfo(QUrlInfo)),
               this, SLOT(ftpListInfo(QUrlInfo)));

    _ftpConnected = false;

#ifndef USE_FTP_CONNECTION_CACHE
    delete _ftp;
#endif

    _ftp = 0;

    return !_ftpConnected;
}

bool FtpFileEngine::atEnd() const
{
    qDebug() << "atEnd()" << _fileName;

    return _fileBuffer.readPos() >= size();
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
FtpFileEngine::beginEntryList(QDir::Filters filters,
                              const QStringList &filterNames)
{
    qDebug() << "beginEntryList() : " << _fileName << _fileFlags;

    _entriesMap.clear();

    FtpFileInfoCache::QUrlInfoList list =
            _ftpCache->findDirInfo(getCachePath(_path));

    if (list.size() > 0)
    {
        FtpFileInfoCache::QUrlInfoListIterator it(list);
        while (it.hasNext())
        {
            QUrlInfo urlInfo = it.next();

            // exclude an empty entry inserted by us and a non-existent entry
            if (urlInfo.isValid() && urlInfo.permissions())
                _entriesMap.insert(urlInfo.name(), urlInfo);
        }
    }
    else if ((_fileFlags & QAbstractFileEngine::DirectoryType)
                && ftpConnect())
    {
        readDir(_path);

        ftpDisconnect();
    }

    return new FtpFileEngineIterator(filters, filterNames,
                                     filterEntries(filters, filterNames,
                                                   _entriesMap));
}

bool FtpFileEngine::caseSensitive() const
{
    qDebug() << "caseSensitive()" << _fileName;

    return true;
}

bool FtpFileEngine::close()
{
    qDebug() << "close()" << _fileName;

    if (_ftpTransfer)
    {
        QEventLoop loop;

        // without this, FTP commands are also blocked
        connect(_ftpTransfer, SIGNAL(finished()), &loop, SLOT(quit()));
        if (_ftpTransfer->isRunning())
            loop.exec();

        _ftpTransfer->wait();

        delete _ftpTransfer;
        _ftpTransfer = 0;
    }

    _fileBuffer.close();

    return true;
}

bool FtpFileEngine::copy(const QString &newName)
{
    Q_UNUSED(newName);

    qDebug() << "copy()" << _fileName;

    return false;
}

QStringList FtpFileEngine::entryList(QDir::Filters filters,
                                     const QStringList &filterNames) const
{
    qDebug() << "entryList()" << _fileName;

    return QAbstractFileEngine::entryList(filters, filterNames);
}

QFile::FileError FtpFileEngine::error() const
{
    qDebug() << "error()" << _fileName;

    return QAbstractFileEngine::error();
}

QString FtpFileEngine::errorString() const
{
    qDebug() << "errorString()" << _fileName;

    return QAbstractFileEngine::errorString();
}

bool FtpFileEngine::extension(Extension extension,
                              const ExtensionOption *option,
                              ExtensionReturn *output)
{
    qDebug() << "extension()" << _fileName;

    return QAbstractFileEngine::extension(extension, option, output);
}

QAbstractFileEngine::FileFlags FtpFileEngine::fileFlags(FileFlags type) const
{
    qDebug() << "fileFlags() : " << _fileName << type;

    // ignore Refresh flag. Qt uses Refresh flag ver frequently,
    // this decrease the cache performance
#if 0
    if (type & QAbstractFileEngine::Refresh)
        const_cast<FtpFileEngine*>(this)->refreshFileInfoCache();
#endif

    qDebug() << "\t" << _fileName << (type & _fileFlags);

    return type & _fileFlags;
}

QString FtpFileEngine::fileName(FileName file) const
{
    qDebug() << "fileName()" << _fileName << file;

    QString result(_url.scheme());

    result.append("://");

    // remove default infomations
    if (_userName != "anonymous")
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

    if (_port != 21)
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
        result.append("?transfermode=").append(_transferMode)
                .append("&encoding=").append(_encoding);
    }

    qDebug() << "\t" << result;

    return result;
}

QDateTime FtpFileEngine::fileTime(FileTime time) const
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

bool FtpFileEngine::flush()
{
    qDebug() << "flush()" << _fileName;

    return _fileBuffer.flush();
}

int FtpFileEngine::handle() const
{
    qDebug() << "handle()" << _fileName;

    return QAbstractFileEngine::handle();
}

bool FtpFileEngine::isRelativePath() const
{
    qDebug() << "isRelativePath()" << _fileName;

    return _url.isRelative();
}

bool FtpFileEngine::isSequential() const
{
    qDebug() << "isSequential()" << _fileName;

    return true;
}

bool FtpFileEngine::link(const QString &newName)
{
    Q_UNUSED(newName);

    qDebug() << "link()" << _fileName;

    return false;
}

uchar* FtpFileEngine::map(qint64 offset, qint64 size,
                          QFile::MemoryMapFlags flags)
{
    qDebug() << "map()" << _fileName;

    return QAbstractFileEngine::map(offset, size, flags);
}

bool FtpFileEngine::mkdir(const QString &dirName,
                          bool createParentDirectories) const
{
    Q_UNUSED(createParentDirectories);

    qDebug() << "mkdir()" << _fileName << dirName;

    FtpFileEngine* This(const_cast<FtpFileEngine*>(this));
    if (!This->ftpConnect())
        return false;

    QUrl url(PathComp::fixUrl(dirName));

    _ftp->mkdir(_textCodec->fromUnicode(url.path()));

    bool result = This->_ftpSync.wait();

    This->ftpDisconnect();

    if (result)
        This->refreshFileInfoCache(url.path());

    return result;
}

bool FtpFileEngine::open(QIODevice::OpenMode openMode)
{
    qDebug() << "open()" << _fileName << openMode;

    if (!(openMode & (QIODevice::ReadOnly | QIODevice::WriteOnly)))
        return false;

    if ((openMode & QIODevice::ReadOnly)
            && (!(_fileFlags & QAbstractFileEngine::ExistsFlag)
                    || _fileFlags & QAbstractFileEngine::DirectoryType))
        return false;

    if ((openMode == QIODevice::WriteOnly)
            && (_fileFlags & QAbstractFileEngine::DirectoryType))
        return false;

#if 0
    close();
#endif

    _fileBuffer.open(QIODevice::ReadWrite);

    if (openMode & QIODevice::ReadOnly)
        _fileBuffer.setSize(_urlInfo.size());

    return true;
}

QString FtpFileEngine::owner(FileOwner owner) const
{
    qDebug() << "owner()" << _fileName << owner;
    qDebug() << "\t" << _urlInfo.owner();

    return QAbstractFileEngine::owner(owner);
}

uint FtpFileEngine::ownerId(FileOwner owner) const
{
    qDebug() << "ownerId() : " << _fileName << owner;

    return QAbstractFileEngine::ownerId(owner);
}

qint64 FtpFileEngine::pos() const
{
    qDebug() << "pos()" << _fileName;

    return 0;
}

qint64 FtpFileEngine::read(char *data, qint64 maxlen)
{
    qDebug() << "read()" << _fileName;

    if (!_ftpTransfer)
    {
        _ftpTransfer = new FtpTransferThread(this, QIODevice::ReadOnly);

        connect(_ftpTransfer, SIGNAL(loopQuit()),
                this, SLOT(abortBufferAccess()));

        qDebug() << "Thread id for read = " << QThread::currentThreadId();
        _ftpTransfer->start();
    }

    return _fileBuffer.read(data, maxlen);
}

qint64 FtpFileEngine::readLine(char *data, qint64 maxlen)
{
    qDebug() << "readLine()" << _fileName;

    return QAbstractFileEngine::readLine(data, maxlen);
}

bool FtpFileEngine::remove()
{
    qDebug() << "remove()" << _fileName;

    if (!ftpConnect())
        return false;

    _ftp->remove(_textCodec->fromUnicode(_path));

    bool result = _ftpSync.wait();

    ftpDisconnect();

    if (result)
    {
        // remove cache entry
        _ftpCache->removeFileInfo(getCachePath(_path));
        _fileFlags = QAbstractFileEngine::FileType;
    }

    return result;
}

bool FtpFileEngine::rename(const QString &newName)
{
    qDebug() << "rename()" << _fileName << _path << newName;

    if (!ftpConnect())
        return false;

    PathComp fixedNewName(newName);
    QString newPath(QUrl(fixedNewName.path()).path());

    _ftp->rename(_textCodec->fromUnicode(_path),
                 _textCodec->fromUnicode(newPath));

    bool result = _ftpSync.wait();

    ftpDisconnect();

    if (result)
    {
        _ftpCache->renameFileInfo(getCachePath(_path), getCachePath(newPath));
        _urlInfo.setName(fixedNewName.fileName());
    }

    return result;
}

bool FtpFileEngine::rmdir(const QString &dirName,
                          bool recurseParentDirectories) const
{
    Q_UNUSED(recurseParentDirectories);

    qDebug() << "rmdir()" << _fileName << dirName;

    FtpFileEngine* This(const_cast<FtpFileEngine*>(this));

    if (!This->ftpConnect())
        return false;

    QUrl url(PathComp::fixUrl(dirName));

    _ftp->rmdir(_textCodec->fromUnicode(url.path()));

    bool result = This->_ftpSync.wait();

    This->ftpDisconnect();

    if (result)
    {
        // remove cache entry
        This->_ftpCache->removeFileInfo(This->getCachePath(url.path()));
        if (_path == url.path())
            This->_fileFlags = QAbstractFileEngine::FileType;
    }

    return result;
}

bool FtpFileEngine::seek(qint64 pos)
{
    Q_UNUSED(pos);

    qDebug() << "seek()" << _fileName;

    return false;
}

void FtpFileEngine::setFileName(const QString &file)
{
    qDebug() << "setFileName() : " << file;

    initFromFileName(file);
}

bool FtpFileEngine::setPermissions(uint perms)
{
    qDebug() << "setPermission()" << _fileName;

    return QAbstractFileEngine::setPermissions(perms);
}

// Set nothing but size. That is, the actual file size does not change.
// So the file is not truncated, and not enrlarged.
bool FtpFileEngine::setSize(qint64 size)
{
    qDebug() << "setSize()" << _fileName << size;

    if (size == -1)
    {
        _fileBuffer.abort();
        _ftpTransfer->abort();
    }
    else
    {
        _fileBuffer.setSize(size);
        _urlInfo.setSize(size);
    }

    return false;
}

qint64 FtpFileEngine::size() const
{
    qDebug() << "size()" << _fileName;
    qDebug() << "\t" << _urlInfo.size();

    return _urlInfo.size();
}

bool FtpFileEngine::supportsExtension(Extension extension) const
{
    qDebug() << "supportsExtension()" << _fileName;

    return QAbstractFileEngine::supportsExtension(extension);
}

bool FtpFileEngine::unmap(uchar *ptr)
{
    qDebug() << "unmap()" << _fileName;

    return QAbstractFileEngine::unmap(ptr);
}

qint64 FtpFileEngine::write(const char *data, qint64 len)
{
    qDebug() << "write()" << _fileName << len;

    if (!_ftpTransfer)
    {
        _ftpTransfer = new FtpTransferThread(this, QIODevice::WriteOnly);

        connect(_ftpTransfer, SIGNAL(loopQuit()),
                this, SLOT(abortBufferAccess()));

        qDebug() << "Thread id for write = " << QThread::currentThreadId();
        _ftpTransfer->start();
    }

    return _fileBuffer.write(data, len);
}

void FtpFileEngine::ftpListInfo(const QUrlInfo &urlInfo)
{
    qDebug() << "ftpFileListInfo() : " << _fileName;
    qDebug() << "\t" << (urlInfo.isDir() ? "[D]" : "[F]") << urlInfo.name();

    // QDir::isReadable() determines with ReadUserPerm
    // And, I think, if ReadOtherPerm is set, assuming ReadUserPerm is set
    // is fine. ftp://hobbes.nmsu.edu set only ReadOtherPerm not ReadUserPerm
    QUrlInfo info(urlInfo);
    if (info.permissions() & QAbstractFileEngine::ReadOtherPerm)
        info.setPermissions(info.permissions() |
                            QAbstractFileEngine::ReadUserPerm);

    // QFtp assumes that a server side file name consists of Latin1 string
    info.setName(_textCodec->toUnicode(info.name().toLatin1()));

    _entriesMap.insert(info.name(), info);
    _ftpCache->addFileInfo(_cacheDir, info);
}

QString FtpFileEngine::getCachePath(const QString& path, bool key)
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

void FtpFileEngine::abortBufferAccess()
{
    _fileBuffer.abort();
}
