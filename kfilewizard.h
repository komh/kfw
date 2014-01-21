#ifndef KFILEWIZARD_H
#define KFILEWIZARD_H

#include <QMainWindow>

#include <QtGui>

#include "entrylistmodel.h"
#include "filesystemsortfilterproxymodel.h"
#include "fileoperation/abstractfileworker.h"

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

    QFileSystemModel* dirModel;
    FileSystemSortFilterProxyModel* dirProxyModel;

    EntryListModel* entryModel;
    FileSystemSortFilterProxyModel* entryProxyModel;

    inline QMessageBox::StandardButton critical(
                const QString& text,
                QMessageBox::StandardButton buttons =
                    QMessageBox::Ok)
    {
        return QMessageBox::critical(this, title(), text, buttons);
    }

    inline QMessageBox::StandardButton question(
                const QString& text,
                QMessageBox::StandardButtons buttons  =
                    QMessageBox::Ok)
    {
        return QMessageBox::question(this, title(), text, buttons);
    }

    void initLocationLine();
    void initSplitter();
    void initDirTree();
    void initEntryTree();

    bool fileWorker(AbstractFileWorker* worker,
                    const QProgressDialog &progress);
    void setLocationText(const QString& text, bool force = false);
    QString canonicalize(const QString& path);
    QString getNameOfCopy(const QString& source);
    QMessageBox::StandardButton checkOverwrite(const QString& dest);
    void setEntryRoot();
    void refreshEntry();

    QString title() const { return tr("K File Wizard"); }
};

#endif // KFILEWIZARD_H
