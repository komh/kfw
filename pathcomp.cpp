/****************************************************************************
**
** PathComp, a class to manipulate path components
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard
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

#include <QUrl>

PathComp::PathComp(const QString& path)
{
    setPath(path);
}

void PathComp::setPath(const QString& path)
{
    _path = fixUrl(QDir::fromNativeSeparators(path));

    int lastSlashIndex = _path.lastIndexOf("/");

    _dir = (lastSlashIndex == -1) ?
                "." :
                (_path.left(lastSlashIndex == 0 ? 1 : lastSlashIndex));

    _fileName = _path.mid(lastSlashIndex + 1);
    // remove url queries
    if (PathComp::isRemotePath(_path))
        _fileName.remove(QRegExp("\\?.*$"));
}

bool PathComp::isRemotePath() const
{
    return PathComp::isRemotePath(path());
}

bool PathComp::isFtpPath() const
{
    return PathComp::isFtpPath(path());
}

bool PathComp::isSFtpPath() const
{
    return PathComp::isSFtpPath(path());
}

bool PathComp::isRoot() const
{
    QString pathOfUrl(QUrl(path()).path());

    return !path().isEmpty() && (pathOfUrl.isEmpty() || pathOfUrl == "/");
}

bool PathComp::isDriveList() const
{
    return path().isEmpty() || (isRemotePath() && QUrl(path()).host().isEmpty());
}

QString PathComp::nativePath() const
{
    if (isRemotePath())
        return path();

    QString nativePath(QDir::toNativeSeparators(path()));

    // convert a drive letter to an upper case
    if (nativePath.length() >= 2 && nativePath.at(0).isLetter()
            && nativePath.at(1) == QLatin1Char(':'))
        nativePath[0] = nativePath.at(0).toUpper();

    return nativePath;
}

QString PathComp::canonicalPath() const
{
    if (isRemotePath())
    {
        QUrl url(path());

        QUrl::FormattingOptions flags(QUrl::None);

        if (isFtpPath() && url.userName() == "anonymous")
            flags |= QUrl::RemoveUserInfo;

        if ((isFtpPath() && url.port() == 21)
                || (isSFtpPath() && url.port() == 22))
            flags |= QUrl::RemovePort;

        flags |= QUrl::RemovePassword | QUrl::RemoveQuery |
                 QUrl::RemoveFragment | QUrl::StripTrailingSlash;

        return url.toString(flags);
    }

    // local path
    return QDir::toNativeSeparators(QFileInfo(path()).absoluteFilePath());
}

bool PathComp::isParentDirOf(const QString &dir) const
{
    QString parentDir(canonicalPath());

    addDirSeparator(parentDir);

    return PathComp(dir).canonicalPath().startsWith(parentDir);
}

bool PathComp::isSubDirOf(const QString &dir) const
{
    QString parentDir(PathComp(dir).canonicalPath());

    addDirSeparator(parentDir);

    return canonicalPath().startsWith(parentDir);
}

QString PathComp::merge(const QString& dir, const QString& fileName)
{
    QString path(QDir::fromNativeSeparators(dir));
    QString fName(QDir::fromNativeSeparators(fileName));

    removeDirSeparator(path);

    while (fName.startsWith("/"))
        fName.remove(0, 1);

    return path.append("/").append(fName);
}

QString PathComp::merge(const QDir& dir, const QString& fileName)
{
    return PathComp::merge(dir.path(), fileName);
}

QString PathComp::fixUrl(const QString &url)
{
    static int colonSlashLength = QString(":/").length();

    // QFileSystemModel does not recognize URL correctly.
    // Always use xxx:/yyy style for a URL as well as a local path
    QString fixedUrl(url);
    int index = fixedUrl.indexOf(":/");
    if (index > 1 && (fixedUrl.endsWith(":/")
                        || fixedUrl.at(index + colonSlashLength) != '/'))
        fixedUrl.replace(index, colonSlashLength, "://");

    return fixedUrl;
}

bool PathComp::isRemotePath(const QString &path)
{
    return isFtpPath(path) || isSFtpPath(path);
}

bool PathComp::isFtpPath(const QString &path)
{
    return QUrl(path).scheme() == "ftp";
}

bool PathComp::isSFtpPath(const QString &path)
{
    return QUrl(path).scheme() == "sftp";
}

QString& PathComp::addDirSeparator(QString& path, bool native)
{
    QString dirSep("/");

    if (native && !isRemotePath(path))
        dirSep = QDir::toNativeSeparators(dirSep);

    if (!path.endsWith(dirSep))
        path.append(dirSep);

    return path;
}

QString& PathComp::removeDirSeparator(QString& path, bool native)
{
    QString dirSep("/");

    if (native && !isRemotePath(path))
        dirSep = QDir::toNativeSeparators(dirSep);

    while (path.endsWith(dirSep))
        path.chop(1);

    return path;
}

QString PathComp::uniquePath(const QString &path, const QString &deco)
{
    QUrl result(path);

    int lastDot = result.path().lastIndexOf(".");

    if (lastDot == -1)
        lastDot = result.path().length();

    QString pathBaseName(result.path().mid(0, lastDot));
    QString suffix(result.path().mid(lastDot + 1 ));

    for (int i = 1;; ++i)
    {
        QString path(pathBaseName);
        if (!deco.isEmpty())
            path.append(deco);
        if (i > 1)
            path.append(QString(" (%1)").arg(i));
        if (!suffix.isEmpty())
        {
            path.append(".");
            path.append(suffix);
        }

        result.setPath(path);
        if (!QFile(result.toString()).exists())
            break;
    }

    return result.toString();
}

QString PathComp::uniqueName(const QString &dir, const QString &name,
                             const QString& deco)
{
    return PathComp(uniquePath(PathComp::merge(dir, name), deco)).fileName();
}
