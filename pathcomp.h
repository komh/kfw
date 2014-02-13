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

#ifndef PATHCOMP_H
#define PATHCOMP_H

#include <QString>
#include <QDir>

class PathComp
{
public:
    PathComp(const QString& path = QString());

    void setPath(const QString& path);

    QString path() const { return _path; }
    QString dir() const { return _dir; }
    QString fileName() const {return _fileName; }

    bool isRemotePath() const;
    bool isFtpPath() const;

    bool isRoot() const;
    bool isDriveList() const;

    QString toNativePath() const;

    static QString merge(const QString& dir, const QString& fileName);
    static QString merge(const QDir& dir, const QString& fileName);
    static QString fixUrl(const QString &url);

    static bool isRemotePath(const QString& path);
    static bool isFtpPath(const QString& path);

private:
    QString _path;
    QString _dir;
    QString _fileName;
};

#endif // PATHCOMP_H
