#include <QtGui>

#include "filesystemsortfilterproxymodel.h"

FileSystemSortFilterProxyModel::FileSystemSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

FileSystemSortFilterProxyModel::~FileSystemSortFilterProxyModel()
{
}

bool FileSystemSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(sourceModel());

    if (model->isDir(left) != model->isDir(right))
        return model->isDir(left);

    QString header(model->headerData(left.column(), Qt::Horizontal).toString());

    if (header == tr("Size") && model->size(left) != model->size(right))
        return model->size(left) < model->size(right);

    if (header == tr("Type") && model->type(left).toUpper() != model->type(right).toUpper())
        return model->type(left).toUpper() < model->type(right).toUpper();

    if (header == tr("Date Modified") && model->lastModified(left) != model->lastModified(right))
        return model->lastModified(left) < model->lastModified(right);

    // Name column and the same sorting index
    return model->fileName(left).toUpper() < model->fileName(right).toUpper();
}
