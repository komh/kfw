/****************************************************************************
**
** FileIconProvider, a class to provider a file icon
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

#include "fileiconprovider.h"

FileIconProvider::FileIconProvider() : QFileIconProvider()
{
}

QIcon FileIconProvider::icon(const QFileInfo &info) const
{
#ifdef Q_WS_PM
    // File type icon is returned for the not accessible files by APIs on OS/2
    // So correct its behavior here
    if (PathComp::isFtpPath(info.filePath()) && !info.isRoot() && info.isDir())
        return QFileIconProvider::icon(QFileIconProvider::Folder);
#endif

    return QFileIconProvider::icon(info);
}
