/****************************************************************************
**
** FtpFileInfoCache, class to cache file informations of FTP servers
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

#include "pathcomp.h"

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
    // if there is already an item, replace it by removing and inserting
    if (findFileInfo(dir, urlInfo.name()).isValid())
        _dirMultiMap.remove(getCacheKey(dir, urlInfo.name()), urlInfo);

    _dirMultiMap.insert(getCacheKey(dir, urlInfo.name()), urlInfo);
}

void FtpFileInfoCache::removeFileInfo(const QString& dir, const QString& name)
{
    QString cacheKey = getCacheKey(dir, name);

    if (_dirMultiMap.contains(cacheKey))
    {
        QUrlInfoListIterator it(_dirMultiMap.values(cacheKey));

        while (it.hasNext())
        {
            QUrlInfo info = it.next();

            if (name == info.name())
            {
                _dirMultiMap.remove(cacheKey, info);
                break;
            }
        }
    }
}

void FtpFileInfoCache::removeFileInfo(const QString& path)
{
    QString dir(PathComp(path).dir());
    QString name(PathComp(path).fileName());

    // Treat root like a file entry
    if (name.isEmpty())
        name = "/";

    removeFileInfo(dir, name);
}

void FtpFileInfoCache::removeDirInfo(const QString& dir)
{
    _dirMultiMap.remove(getCacheKey(dir));
}

void FtpFileInfoCache::renameFileInfo(const QString &dir,
                                      const QString &name,
                                      const QString &newDir,
                                      const QString &newName)
{
    QUrlInfo info(findFileInfo(dir, name));

    if (!info.isValid())
        return;

    removeFileInfo(dir, name);

    info.setName(newName);

    addFileInfo(newDir, info);
}

void FtpFileInfoCache::renameFileInfo(const QString &path,
                                      const QString &newPath)
{
    QString dir(PathComp(path).dir());
    QString name(PathComp(path).fileName());

    // Treat root like a file entry
    if (name.isEmpty())
        name = "/";

    QString newDir(PathComp(newPath).dir());
    QString newName(PathComp(newPath).fileName());

    // Treat root like a file entry
    if (newName.isEmpty())
        newName = "/";

    renameFileInfo(dir, name, newDir, newName);
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
    QString dir(PathComp(path).dir());
    QString name(PathComp(path).fileName());

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
