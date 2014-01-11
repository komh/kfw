#ifndef FILESYSTEMSORTFILTERPROXYMODEL_H
#define FILESYSTEMSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class FileSystemSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileSystemSortFilterProxyModel(QObject *parent = 0);
    ~FileSystemSortFilterProxyModel();

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // FILESYSTEMSORTFILTERPROXYMODEL_H
