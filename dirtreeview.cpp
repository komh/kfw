/****************************************************************************
**
** DirTreeView, a class to show a directory tree
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

#include "dirtreeview.h"

#include <QDragEnterEvent>
#include <QFileSystemModel>
#include <QApplication>
#include <QDebug>

#include "filesystemsortfilterproxymodel.h"
#include "urllistmimedata.h"
#include "pathcomp.h"

DirTreeView::DirTreeView(QWidget *parent) :
    QTreeView(parent)
{
    setAcceptDrops(true);
    setAutoScroll(true);
    setAutoExpandDelay(1000);
}

void DirTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(UrlListMimeData::format()))
    {
        // set to DraggingState to enable auto expanding
        setState(QAbstractItemView::DraggingState);

        event->setDropAction(determineDropAction(event->pos(),
                                                 event->keyboardModifiers(),
                                                 event->mimeData()));
        event->accept();

        return;
    }

    event->ignore();
}

void DirTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeView::dragMoveEvent(event);

    if (event->mimeData()->hasFormat(UrlListMimeData::format()))
    {
        event->setDropAction(determineDropAction(event->pos(),
                                                 event->keyboardModifiers(),
                                                 event->mimeData()));
        event->accept();

        return;
    }

    event->ignore();
}

void DirTreeView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(UrlListMimeData::format()))
    {
        FileSystemSortFilterProxyModel* proxyModel =
                qobject_cast<FileSystemSortFilterProxyModel*>(model());
        QFileSystemModel* dirModel =
                qobject_cast<QFileSystemModel*>(proxyModel->sourceModel());

        emit dropped(UrlListMimeData::listFrom(event->mimeData()),
                     dirModel->filePath(
                         proxyModel->mapToSource(indexAt(event->pos()))),
                     event->dropAction() == Qt::CopyAction);

        event->setDropAction(event->dropAction());
        event->accept();

        return;
    }

    event->ignore();
}

Qt::DropAction DirTreeView::determineDropAction(
        const QPoint &pos, const Qt::KeyboardModifiers& modifiers,
        const QMimeData *mimeData)
{
    QList<QUrl> urlList(UrlListMimeData::listFrom(mimeData));

    FileSystemSortFilterProxyModel* proxyModel =
            qobject_cast<FileSystemSortFilterProxyModel*>(model());
    QFileSystemModel* dirModel =
            qobject_cast<QFileSystemModel*>(proxyModel->sourceModel());

    QString dir =
            dirModel->filePath(proxyModel->mapToSource(indexAt(pos)));

    QString first = urlList.first().toString();

    // same directory
    if (dir == PathComp(first).dir())
        return Qt::IgnoreAction;

    // do not move remote entries
    if (PathComp::isRemotePath(dir))
        return Qt::CopyAction;

    if (modifiers & Qt::ControlModifier)
        return Qt::CopyAction;

    // same local drives
    if (dir.length() >= 2  && dir.left(2).toUpper() ==  first.left(2).toUpper()
            && dir.at(1) == ':')
        return Qt::MoveAction;

    return Qt::CopyAction;
}
