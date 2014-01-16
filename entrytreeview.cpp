#include <QtGui>

#include "entrytreeview.h"
#include "entrylistmodel.h"
#include "filesystemsortfilterproxymodel.h"
#include "fileoperation.h"

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

void EntryTreeView::copyToClipboard()
{
    qDebug() << "copyToClipboard()";

    QList<QUrl> urlList;

    QModelIndexList selected(selectedIndexes());

    if (selected.size() == 0)
        return;

    foreach (QModelIndex proxyIndex, selected)
    {
        FileSystemSortFilterProxyModel* proxyModel = qobject_cast<FileSystemSortFilterProxyModel*>(model());
        EntryListModel* sourceModel = qobject_cast<EntryListModel*>(proxyModel->sourceModel());

        QModelIndex index = proxyModel->mapToSource(proxyIndex);

        if (sourceModel->headerData(index.column(), Qt::Horizontal) == tr("Name"))
            urlList.append(FileOperation::fixUrl(sourceModel->filePath(index)));
    }

    qDebug() << "\t" << urlList;

    QMimeData *mime = new QMimeData;
    mime->setUrls(urlList);

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setMimeData(mime);
}

void EntryTreeView::pasteFromClipboard()
{
    qDebug() << "pasteFromClipboard()";

    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mime = clipboard->mimeData();

    if (mime->hasUrls())
    {
        qDebug() << "\t" << mime->urls();

        emit paste(mime->urls());
    }
}

void EntryTreeView::deletePressed()
{
    qDebug() << "deletePressed()";

    QList<QUrl> urlList;

    QModelIndexList selected(selectedIndexes());

    if (selected.size() == 0)
        return;

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

    emit remove(urlList);
}
