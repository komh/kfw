/****************************************************************************
**
** EntryListModel, subclass of QFileSystemModel for EntryTreeView
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

#include "entrylistmodel.h"

EntryListModel::EntryListModel(QObject *parent) :
    QFileSystemModel(parent)
{
}

bool EntryListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    emit renameBegin(filePath(index), value.toString());

    bool result = QFileSystemModel::setData(index, value, role);

    emit renameEnd();

    return result;
}
