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

#include <QDir>

#include "pathcomp.h"

PathComp::PathComp(const QString& path)
{
    setPath(path);
}

void PathComp::setPath(const QString& path)
{
    _path = QDir::fromNativeSeparators(path);

    int lastSlashIndex = path.lastIndexOf("/");

    _dir = (lastSlashIndex == -1) ?
                "." :
                (path.left(lastSlashIndex == 0 ? 1 : lastSlashIndex));

    _fileName = path.mid(lastSlashIndex + 1);
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
