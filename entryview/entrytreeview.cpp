/****************************************************************************
**
** EntryTreeView, class in order to show directory entires
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

#include "entrytreeview.h"
#include "entrylistmodel.h"
#include "filesystemsortfilterproxymodel.h"
#include "pathcomp.h"
#include "urllistmimedata.h"
#include "qttr.h"

EntryTreeView::EntryTreeView(QWidget *parent) :
    QTreeView(parent)
  , popupMenu(this)
{
    setAcceptDrops(true);
}

void EntryTreeView::dragEnterEvent(QDragEnterEvent *event)
{
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

void EntryTreeView::dragMoveEvent(QDragMoveEvent *event)
{
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

void EntryTreeView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(UrlListMimeData::format()))
    {
        FileSystemSortFilterProxyModel* proxyModel =
                qobject_cast<FileSystemSortFilterProxyModel*>(model());
        EntryListModel* entryModel =
                qobject_cast<EntryListModel*>(proxyModel->sourceModel());

        QModelIndex index = proxyModel->mapToSource(indexAt(event->pos()));
        if (!index.isValid() || !entryModel->isDir(index))
            index = proxyModel->mapToSource(rootIndex());

        QString to(entryModel->filePath(index));

        emit dropped(UrlListMimeData::listFrom(event->mimeData()), to,
                     event->dropAction() == Qt::CopyAction);

        event->setDropAction(event->dropAction());
        event->accept();

        return;
    }
    
    event->ignore();
}

void EntryTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button())
        startPos = event->pos();

    QTreeView::mousePressEvent(event);
}

void EntryTreeView::mouseMoveEvent(QMouseEvent *event)
{
    int distance = (event->pos() - startPos).manhattanLength();

    if ((event->buttons())
            && distance >= QApplication::startDragDistance())
        perfromDrag();

    // Do not call QTreeView::mouseMoveEvent().
    // Calling it causes selection to be changed
}

void EntryTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        popupMenu.clear();
        popupMenu.addAction(tr("Copy"), this, SLOT(copy()),
                            QKeySequence(QKeySequence::Copy));
        popupMenu.addAction(tr("Cut"), this, SLOT(cut()),
                            QKeySequence(QKeySequence::Cut));

        QClipboard* clipboard = QApplication::clipboard();
        const QMimeData* mime = clipboard->mimeData();
        if (mime->hasFormat(
                    UrlListMimeData::format(UrlListMimeData::CopyAction))
            || mime->hasFormat(
                    UrlListMimeData::format(UrlListMimeData::CutAction)))
            popupMenu.addAction(tr("Paste"), this, SLOT(paste()),
                                QKeySequence(QKeySequence::Paste));
        popupMenu.addSeparator();

        popupMenu.addAction(tr("Delete"), this, SLOT(remove()),
                            QKeySequence(QKeySequence::Delete));
        popupMenu.addAction(tr("Rename"), this, SLOT(rename()),
                            QKeySequence(Qt::Key_F2));
        popupMenu.addAction(tr("Create a directory"), this, SLOT(mkdir()));
        popupMenu.addSeparator();

        popupMenu.addAction(tr("Refresh"), this, SIGNAL(refresh()),
                            QKeySequence(QKeySequence::Refresh));

        popupMenu.popup(event->globalPos());
    }

    QTreeView::mouseReleaseEvent(event);
}

void EntryTreeView::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Back))
    {
        emit cdUp(rootIndex());

        return;
    }

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

    if (event->key() == Qt::Key_F2)
    {
        rename();

        return;
    }

    if (event->matches(QKeySequence::Refresh))
    {
        emit refresh();

        return;
    }

    QTreeView::keyPressEvent(event);
}

void EntryTreeView::copyToClipboard(bool copy)
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

void EntryTreeView::pasteFromClipboard()
{
    qDebug() << "pasteFromClipboard()";

    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mime = clipboard->mimeData();
    if (mime->hasFormat(UrlListMimeData::format(UrlListMimeData::CopyAction)))
    {
        qDebug() << "pasteFromClipboard()"
                 << "Copy list" << UrlListMimeData::listFrom(mime);

        emit paste(UrlListMimeData::listFrom(mime), QString());
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
                   QString(), false);

        clipboard->clear();
    }
}

void EntryTreeView::deletePressed()
{
    qDebug() << "deletePressed()";

    QList<QUrl> urlList(selectedUrlList());

    if (urlList.size() == 0)
        return;

    emit remove(urlList);
}

QList<QUrl> EntryTreeView::selectedUrlList()
{
    qDebug() << "selectedUrlList()";

    QList<QUrl> urlList;

    QModelIndexList selected(selectedIndexes());

    foreach (QModelIndex proxyIndex, selected)
    {
        FileSystemSortFilterProxyModel* proxyModel =
                qobject_cast<FileSystemSortFilterProxyModel*>(model());
        EntryListModel* sourceModel =
                qobject_cast<EntryListModel*>(proxyModel->sourceModel());

        QModelIndex index = proxyModel->mapToSource(proxyIndex);

        if (sourceModel->headerData(index.column(), Qt::Horizontal)
                == QtTr::name())
        {
            // use canonicalFilePath() to include URL queries
            urlList.append(QFileInfo(sourceModel->filePath(index))
                            .canonicalFilePath());
        }
    }

    qDebug() << "\t" << urlList;

    return urlList;
}

void EntryTreeView::perfromDrag()
{
    QList<QUrl> urlList(selectedUrlList());

    if (urlList.size() == 0)
        return;

    UrlListMimeData* mimeData = new UrlListMimeData;
    mimeData->setList(urlList);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
}

Qt::DropAction EntryTreeView::determineDropAction(
        const QPoint& pos, const Qt::KeyboardModifiers &modifiers,
        const QMimeData* mimeData)
{
    QList<QUrl> urlList(UrlListMimeData::listFrom(mimeData));

    QString first = urlList.first().toString();

    FileSystemSortFilterProxyModel* proxyModel =
            qobject_cast<FileSystemSortFilterProxyModel*>(model());
    EntryListModel* entryModel =
            qobject_cast<EntryListModel*>(proxyModel->sourceModel());

    QModelIndex index = proxyModel->mapToSource(indexAt(pos));
    bool isIndexDir = index.isValid() && entryModel->isDir(index);

    QString targetDir = isIndexDir ?
                entryModel->filePath(index) :
                entryModel->filePath(proxyModel->mapToSource(rootIndex()));

    // drive list or ftp list
    if (PathComp(targetDir).isDriveList())
        return Qt::IgnoreAction;

    // same directory
    if (targetDir == PathComp(first).dir())
        return Qt::IgnoreAction;

    if (modifiers & Qt::ControlModifier)
        return Qt::CopyAction;

    // same local drives
    if (targetDir.length() >= 2
            && targetDir.left(2).toUpper() ==  first.left(2).toUpper()
            && targetDir.at(1) == ':')
        return Qt::MoveAction;

    return Qt::CopyAction;
}

void EntryTreeView::copy()
{
    copyToClipboard();
}

void EntryTreeView::cut()
{
    copyToClipboard(false);
}

void EntryTreeView::paste()
{
    pasteFromClipboard();
}

void EntryTreeView::remove()
{
    deletePressed();
}

void EntryTreeView::rename()
{
    QModelIndex index = currentIndex();

    edit(index);
}

void EntryTreeView::mkdir()
{
    FileSystemSortFilterProxyModel* proxyModel =
            qobject_cast<FileSystemSortFilterProxyModel*>(model());
    EntryListModel* entryModel =
            qobject_cast<EntryListModel*>(proxyModel->sourceModel());

    QString newDir(PathComp::uniqueName(entryModel->rootPath(),
                                        tr("New Dir")));

    QModelIndex index(entryModel->mkdir(proxyModel->mapToSource(rootIndex()),
                                        newDir));

    if (!index.isValid())
        return;

    index = proxyModel->mapFromSource(index);
    edit(index);

    setCurrentIndex(index);
}
