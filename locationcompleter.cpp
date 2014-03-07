/****************************************************************************
**
** LocationCompleter, a completer for a location
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

#include "locationcompleter.h"

#include "pathcomp.h"

LocationCompleter::LocationCompleter(QObject *parent) :
    QCompleter(parent)
{
}

QString LocationCompleter::pathFromIndex(const QModelIndex &index) const
{
    // do not convert separators of FTP path to back-slash
    return PathComp(QCompleter::pathFromIndex(index)).nativePath();
}

QStringList LocationCompleter::splitPath(const QString &path) const
{
    QString qtPath(path);

    // QFileSystemModel stores paths in ftp:/ style not in ftp://
    qtPath.replace("://", ":/");

    return QCompleter::splitPath(qtPath);
}
