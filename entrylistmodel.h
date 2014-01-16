#ifndef ENTRYLISTMODEL_H
#define ENTRYLISTMODEL_H

#include <QFileSystemModel>

class EntryListModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit EntryListModel(QObject* parent = 0);

    bool hasChildren(const QModelIndex &parent) const
    {
        return QFileSystemModel::hasChildren(parent) && parent == _rootIndex;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags itemFlags = QFileSystemModel::flags(index);

        if (headerData(index.column(), Qt::Horizontal).toString()
                == tr("Name"))
            itemFlags |= Qt::ItemIsEditable;

        return itemFlags;
    }

    void setRootIndex(const QModelIndex rootIndex)
    {
        _rootIndex = rootIndex;
    }

private:
    QModelIndex _rootIndex;
};

#endif // ENTRYLISTMODEL_H
