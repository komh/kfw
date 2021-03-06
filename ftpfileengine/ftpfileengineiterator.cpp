/****************************************************************************
**
** FtpFileEngineIterator, class to iterate FTP entries
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

#include "ftpfileengineiterator.h"

FtpFileEngineIterator::FtpFileEngineIterator(QDir::Filters filters,
                                             const QStringList& nameFilters,
                                             const QStringList& entries)
    : QAbstractFileEngineIterator(filters, nameFilters)
    , _entries(entries)
    , _index(-1)
{
}

bool FtpFileEngineIterator::hasNext() const
{
    return _index < _entries.size() - 1;
}

QString FtpFileEngineIterator::next()
{
    if (!hasNext())
        return QString();

    ++_index;

    return currentFilePath();
}

QString FtpFileEngineIterator::currentFileName() const
{
    return _entries.at(_index);
}
