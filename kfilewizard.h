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

    bool eventFilter(QObject* target, QEvent *event);

private slots:
    void dirLoaded(const QString& dir);
    void dirActivated(const QModelIndex& index);
    void entryActivated(const QModelIndex& index);
    void entryCdUp(const QModelIndex &index);
    void entryPaste(const QList<QUrl>& urlList);
    void entryRemove(const QList<QUrl>& urlList);
    void locationReturnPressed(bool moveFocusToEntryView = true);

private:
    Ui::KFileWizard *ui;

    QDir currentDir;

    bool locationMouseFocus;

    QFileSystemModel* dirModel;
    FileSystemSortFilterProxyModel* dirProxyModel;

    EntryListModel* entryModel;
    FileSystemSortFilterProxyModel* entryProxyModel;

    inline QMessageBox::StandardButton critical(
                const QString& text,
                QMessageBox::StandardButton buttons =
                    QMessageBox::Ok)
    {
        return QMessageBox::critical(this, tr("K File Wizard"), text,
                                     buttons);
    }

    inline QMessageBox::StandardButton question(
                const QString& text,
                QMessageBox::StandardButtons buttons  =
                    QMessageBox::Ok)
    {
        return QMessageBox::question(this, tr("K File Wizard"), text,
                                     buttons);
    }

    void initLocationLine();
    void initSplitter();
    void initDirTree();
    void initEntryTree();

    void setLocationText(const QString& text, bool force = false);
    QString canonicalize(const QString& path);
    QString getNameOfCopy(const QString& source);
    QMessageBox::StandardButton checkOverwrite(const QString& dest);
    void setEntryRoot();
};

#endif // KFILEWIZARD_H
