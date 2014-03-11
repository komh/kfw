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
#include <QClipboard>

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

void DirTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button())
        startPos = event->pos();

    QTreeView::mousePressEvent(event);
}

void DirTreeView::mouseMoveEvent(QMouseEvent *event)
{
    int distance = (event->pos() - startPos).manhattanLength();

    if ((event->buttons())
            && distance >= QApplication::startDragDistance())
        performDrag();

    // Do not call QTreeView::mouseMoveEvent().
    // Calling it causes selection to be changed
}

void DirTreeView::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy))
    {
        copyToClipboard();

        return;
    }

    if (event->matches(QKeySequence::Cut))
    {
        copyToClipboard(false);

        return;
    }

    if (event->matches(QKeySequence::Paste))
    {
        pasteFromClipboard();

        return;
    }

    if (event->matches(QKeySequence::Delete))
    {
        deletePressed();

        return;
    }

    QTreeView::keyPressEvent(event);
}

void DirTreeView::copyToClipboard(bool copy)
{
    qDebug() << "copyToClipboard()";

    QList<QUrl> urlList(selectedUrlList());

    if (urlList.size() == 0)
        return;

    UrlListMimeData* mime =
            new UrlListMimeData(copy ? UrlListMimeData::CopyAction :
                                       UrlListMimeData::CutAction);
    mime->setList(urlList);

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setMimeData(mime);
}

void DirTreeView::pasteFromClipboard()
{
    qDebug() << "pasteFromClipboard()";

    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mime = clipboard->mimeData();
    if (mime->hasFormat(UrlListMimeData::format(UrlListMimeData::CopyAction)))
    {
        qDebug() << "pasteFromClipboard()"
                 << "Copy list" << UrlListMimeData::listFrom(mime);

        emit paste(UrlListMimeData::listFrom(mime));
    }
    else
    if (mime->hasFormat(UrlListMimeData::format(UrlListMimeData::CutAction)))
    {
        qDebug() << "pasteFromClipboard()"
                 << "Cut list"
                 << UrlListMimeData::listFrom(mime,
                                              UrlListMimeData::CutAction);

        emit paste(UrlListMimeData::listFrom(mime,
                                             UrlListMimeData::CutAction),
                   false);

        clipboard->clear();
    }
}

void DirTreeView::deletePressed()
{
    qDebug() << "deletePressed()";

    QList<QUrl> urlList(selectedUrlList());

    if (urlList.size() == 0)
        return;

    emit remove(urlList);
}

QList<QUrl> DirTreeView::selectedUrlList()
{
    qDebug() << "selectedUrlList()";

    QList<QUrl> urlList;

    QModelIndexList selected(selectedIndexes());

    FileSystemSortFilterProxyModel* proxyModel =
            qobject_cast<FileSystemSortFilterProxyModel*>(model());
    QFileSystemModel* sourceModel =
            qobject_cast<QFileSystemModel*>(proxyModel->sourceModel());

    foreach (QModelIndex proxyIndex, selected)
    {
        QModelIndex index = proxyModel->mapToSource(proxyIndex);

        urlList.append(PathComp::fixUrl(sourceModel->filePath(index)));
    }

    qDebug() << "\t" << urlList;

    return urlList;
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

    // drive list
    if (PathComp(dir).isDriveList())
        return Qt::IgnoreAction;

    // same directory
    if (dir == PathComp(first).dir())
        return Qt::IgnoreAction;

    if (modifiers & Qt::ControlModifier)
        return Qt::CopyAction;

    // same local drives
    if (dir.length() >= 2  && dir.left(2).toUpper() ==  first.left(2).toUpper()
            && dir.at(1) == ':')
        return Qt::MoveAction;

    return Qt::CopyAction;
}

void DirTreeView::performDrag()
{
    QList<QUrl> urlList(selectedUrlList());

    if (urlList.isEmpty())
        return;

    UrlListMimeData* mimeData = new UrlListMimeData;
    mimeData->setList(urlList);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
}
