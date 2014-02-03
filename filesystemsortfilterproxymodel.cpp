/****************************************************************************
**
** FileSystemSortFilterProxyModel, class in order to sort QFileSystemModel
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

#include <QtGui>

#include "filesystemsortfilterproxymodel.h"

#include "qttr.h"

FileSystemSortFilterProxyModel::FileSystemSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

FileSystemSortFilterProxyModel::~FileSystemSortFilterProxyModel()
{
}

bool FileSystemSortFilterProxyModel::lessThan(const QModelIndex &left,
                                              const QModelIndex &right) const
{
    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(sourceModel());

    if (model->isDir(left) != model->isDir(right))
        return model->isDir(left);

    QString header(model->headerData(left.column(),
                                     Qt::Horizontal).toString());

    if (header == QtTr::size() && model->size(left) != model->size(right))
        return model->size(left) < model->size(right);

    if (header == QtTr::type()
            && model->type(left).toUpper()  !=  model->type(right).toUpper())
        return model->type(left).toUpper() < model->type(right).toUpper();

    if (header == QtTr::dateModified()
            && model->lastModified(left) != model->lastModified(right))
        return model->lastModified(left) < model->lastModified(right);

    // Name column and the same sorting index

    // Place ftp: to the end
    if (model->fileName(left) == "ftp:")
        return false;

    if (model->fileName(right) == "ftp:")
        return true;

    return model->filePath(left).toUpper() < model->filePath(right).toUpper();
}
