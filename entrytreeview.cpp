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
#include "fileoperation/fileoperation.h"
#include "urllistmimedata.h"

EntryTreeView::EntryTreeView(QWidget *parent) :
    QTreeView(parent)
{
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
                == tr("Name"))
            urlList.append(FileOperation::fixUrl(
                               sourceModel->filePath(index)));
    }

    qDebug() << "\t" << urlList;

    return urlList;
}
