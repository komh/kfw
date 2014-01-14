#ifndef KFILEWIZARD_H
#define KFILEWIZARD_H

#include <QMainWindow>

#include <QtGui>

#include "entrylistmodel.h"
#include "filesystemsortfilterproxymodel.h"

namespace Ui {
class KFileWizard;
}

class KFileWizard : public QMainWindow
{
    Q_OBJECT

public:
    explicit KFileWizard(QWidget *parent = 0);
    ~KFileWizard();

private slots:
    void dirLoaded(const QString& dir);
    void dirActivated(const QModelIndex& index);
    void entryActivated(const QModelIndex& index);
    void entryCdUp(const QModelIndex &index);
    void locationReturnPressed();

private:
    Ui::KFileWizard *ui;

    QDir currentDir;

    QFileSystemModel* dirModel;
    FileSystemSortFilterProxyModel* dirProxyModel;

    EntryListModel* entryModel;
    FileSystemSortFilterProxyModel* entryProxyModel;

    inline void critical(const QString& text)
    {
        QMessageBox::critical(this, tr("K File Wizard"), text, QMessageBox::Ok);
    }

    void initLocationLine();
    void initSplitter();
    void initDirTree();
    void initEntryTree();

    void setLocationText(const QString& text, bool force = false);
    QString canonicalize(const QString& path);
    void setEntryRoot();
};

#endif // KFILEWIZARD_H
