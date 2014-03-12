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

#ifndef ENTRYLISTMODEL_H
#define ENTRYLISTMODEL_H

#include <QFileSystemModel>

#include "pathcomp.h"
#include "qttr.h"

class EntryListModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit EntryListModel(bool entryView = true, QObject* parent = 0);

    bool hasChildren(const QModelIndex &parent) const
    {
        return QFileSystemModel::hasChildren(parent)
                && (!_entryView || parent == _rootIndex);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags itemFlags = QFileSystemModel::flags(index);

        PathComp pathComp(filePath(index));

        if (pathComp.isDriveList() || pathComp.isRoot())
            itemFlags &= ~Qt::ItemIsEditable;
        else
        if (headerData(index.column(), Qt::Horizontal).toString()
                == QtTr::name())
            itemFlags |= Qt::ItemIsEditable;

        return itemFlags;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QString filePath(const QModelIndex &index) const
    {
        return PathComp::fixUrl(QFileSystemModel::filePath(index));
    }

    QString rootPath() const
    {
        return PathComp::fixUrl(QFileSystemModel::rootPath());
    }

    void setRootIndex(const QModelIndex rootIndex)
    {
        _rootIndex = rootIndex;
    }

signals:
    void renameBegin(const QString& oldName, const QString& newName);
    void renameEnd(bool result);

private:
    QModelIndex _rootIndex;
    bool _entryView;
};

#endif // ENTRYLISTMODEL_H
