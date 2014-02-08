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
}

bool PathComp::isRemotePath() const
{
    return PathComp::isRemotePath(path());
}

bool PathComp::isFtpPath() const
{
    return PathComp::isFtpPath(path());
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

QString PathComp::merge(const QString& dir, const QString& fileName)
{
    QString path(QDir::fromNativeSeparators(dir));
    QString fName(QDir::fromNativeSeparators(fileName));

    while (path.endsWith("/"))
        path.chop(1);

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
    return isFtpPath(path);
}

bool PathComp::isFtpPath(const QString &path)
{
    return QUrl(path).scheme() == "ftp";
}
