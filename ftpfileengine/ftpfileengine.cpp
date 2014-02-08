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

    // delete causes ASSERT to be failed on Windows debug build
    _ftp->deleteLater();
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

    _url.setUserName(_userName);
    _url.setPassword(_password);
    _url.setPort(_port);

    hostCache.addHostInfo(_url.host(), _userName, _password, _port);

    _path = _url.path();
    if (_path.isEmpty())
        _path = "/";

    while (_path.length() > 1 && _path.endsWith("/"))
        _path.chop(1);

    _fileName = _url.scheme().append("://").append(_url.host()).append(_path);
}

void FtpFileEngine::initFtp()
{
    _ftp = new QFtp;
    connect(_ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(ftpListInfo(QUrlInfo)));

    _ftpSync.setFtp(_ftp);

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

void FtpFileEngine::refreshFileInfoCache()
{
    // ftp: case ?
    if (_url.host().isEmpty())
    {
        qDebug() << "refreshFileInfoCache()" << "host empty" << _path;

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

        _cacheDir = getCachePath(dir, true);

        _ftpCache->removeDirInfo(getCachePath(dir));

        _entries.clear();
        _entriesMap.clear();

        // get a file list from a parent directory
        _ftp->cd(dir);
        _ftp->list();

        _ftpSync.wait();

        _urlInfo = _entriesMap.value(name);

        _fileFlags = 0;

        if (_entries.contains(name))
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

bool FtpFileEngine::ftpConnect()
{
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

    return _ftpConnected;
}

bool FtpFileEngine::ftpDisconnect()
{
    if (!_ftpConnected)
        return true;

    _ftp->close();

    _ftpConnected = !_ftpSync.wait();

    return _ftpConnected;
}

bool FtpFileEngine::atEnd() const
{
    qDebug() << "atEnd()" << _fileName;

    return _fileBuffer.readPos() >= size();
}

QAbstractFileEngine::Iterator*
FtpFileEngine::beginEntryList(QDir::Filters filters,
                              const QStringList &filterNames)
{
    qDebug() << "beginEntryList() : " << _fileName << _fileFlags;

    _entries.clear();
    _entriesMap.clear();

    FtpFileInfoCache::QUrlInfoList list =
            _ftpCache->findDirInfo(getCachePath(_path));

    if (list.size() > 0)
    {
        FtpFileInfoCache::QUrlInfoListIterator it(list);
        while (it.hasNext())
        {
            QUrlInfo urlInfo = it.next();

            // exclude an empty entry inserted by us
            if (urlInfo.isValid())
            {
                _entries.append(urlInfo.name());
                _entriesMap.insert(urlInfo.name(), urlInfo);
            }
        }
    }
    else if ((_fileFlags & QAbstractFileEngine::DirectoryType)
                && ftpConnect())
    {
        _cacheDir = getCachePath(_path, true);

        // add an empty entry to distinguish a empty directory from
        // a non-existent directory
        _ftpCache->addFileInfo(_cacheDir, QUrlInfo());

        _ftp->cd(_path);
        _ftp->list();

        ftpDisconnect();
    }

    return new FtpFileEngineIterator(filters, filterNames, _entries);
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
        result.append(_userName).append("@");

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
    qDebug() << "mkdir()" << _fileName;

    return QAbstractFileEngine::mkdir(dirName, createParentDirectories);
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

    _ftp->remove(_path);

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

    QString newPath(QUrl(PathComp::fixUrl(newName)).path());

    _ftp->rename(_path, newPath);

    bool result = _ftpSync.wait();

    ftpDisconnect();

    if (result)
    {
        _ftpCache->renameFileInfo(getCachePath(_path), newName);
        _urlInfo.setName(PathComp(newPath).fileName());
    }

    return result;
}

bool FtpFileEngine::rmdir(const QString &dirName,
                          bool recurseParentDirectories) const
{
    qDebug() << "rmdir()" << _fileName;

    return QAbstractFileEngine::rmdir(dirName, recurseParentDirectories);
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

    _entries.append(urlInfo.name());

    // QDir::isReadable() determines with ReadUserPerm
    // And, I think, if ReadOtherPerm is set, assuming ReadUserPerm is set
    // is fine. ftp://hobbes.nmsu.edu set only ReadOtherPerm not ReadUserPerm
    QUrlInfo info(urlInfo);
    if (info.permissions() & QAbstractFileEngine::ReadOtherPerm)
        info.setPermissions(info.permissions() |
                            QAbstractFileEngine::ReadUserPerm);

    _entriesMap.insert(urlInfo.name(), info);

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
