/****************************************************************************
**
** KFileWizard, the main class of K File Wizard
** Copyright (C) 2014 by KO Myung-Hun
** All rights reserved.
** Contact: KO Myung-Hun (komh@chollian.net)
**
** This file is part of K File Wizard.
**
** $BEGIN_LICENSE$
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $END_LICENSE$
**
****************************************************************************/

#ifndef KFILEWIZARD_H
#define KFILEWIZARD_H

#include <QMainWindow>

#include <QtGui>

#include "entryview/entrylistmodel.h"
#include "filesystemsortfilterproxymodel.h"
#include "fileoperation/abstractfileworker.h"
#include "delayedmessagebox.h"

#include "sharedmemory.h"

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

    void lazyInitGeometry();

protected:
    bool event(QEvent *event);
    void closeEvent(QCloseEvent* event);
    void moveEvent(QMoveEvent* event);
    void resizeEvent(QResizeEvent* event);

private slots:
    void appFocusChanged(QWidget* old, QWidget* now);
    void dirLoaded(const QString& dir);
    void dirActivated(const QModelIndex& index);
    void dirDropped(const QList<QUrl>& urlList, const QString& to, bool copy);
    void entryActivated(const QModelIndex& index);
    void entryCdUp(const QModelIndex &index);
    void entryPaste(const QList<QUrl>& urlList, const QString& to, bool copy);
    void entryRemove(const QList<QUrl>& urlList);
    void entryRefresh();
    void renameBegin(const QString& oldName, const QString& newName);
    void renameEnd(bool success);
    void locationReturnPressed(bool focusToEntry = true, bool bySignal = true);
    void about();
    void connectTo();
    void openAddressBook();

private:
    Ui::KFileWizard *ui;

    QDir currentDir;

    QFileSystemModel* dirModel;
    FileSystemSortFilterProxyModel* dirProxyModel;

    EntryListModel* entryModel;
    FileSystemSortFilterProxyModel* entryProxyModel;

    QFileSystemModel* locationCompleterModel;

    DelayedMessageBox delayedMsgBox;

    SharedMemory sharedMem;
    QPoint initialPos;
    QSize initialSize;

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

    void initMenu();
    void initLocationLine();
    void initSplitter();
    void initDirTree();
    void initEntryTree();
    void initEntryModel();

    void copyUrlsTo(const QList<QUrl>& urlList, const QString& to,
                    bool copy = true);

    void removeUrls(const QList<QUrl>& urlList);

    bool fileWorker(AbstractFileWorker* worker,
                    QProgressDialog *progress);
    void setLocationText(const QString& text, bool focusToEntry = false);
    QString canonicalize(const QString& path);
    QModelIndex findDirIndex(const QString& dir);
    QString getNameOfCopy(const QString& source);
    QMessageBox::StandardButton checkOverwrite(QProgressDialog* progress,
                                               const QString& dest);
    void setEntryRoot();
    QString newPathForRemove(const QList<QUrl> &urlList);
    void refreshEntryModel(bool force);
    void selectEntries(const QList<QUrl> &urlListToSelect, bool remove);
    void refreshEntry(const QList<QUrl>& urlList, bool remove = false,
                      bool force = false);

    QString organization() const { return tr("KO Myung-Hun"); }
    QString title() const { return tr("K File Wizard"); }
    QString version() const { return tr("1.1.0"); }

    void saveSettings();
    void loadSettings();
};

#endif // KFILEWIZARD_H
