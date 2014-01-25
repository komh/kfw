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

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void appFocusChanged(QWidget* old, QWidget* now);
    void dirLoaded(const QString& dir);
    void dirActivated(const QModelIndex& index);
    void entryActivated(const QModelIndex& index);
    void entryCdUp(const QModelIndex &index);
    void entryPaste(const QList<QUrl>& urlList, bool copy);
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
    void setLocationText(const QString& text);
    QString canonicalize(const QString& path);
    QModelIndex findDirIndex(const QString& dir);
    QString getNameOfCopy(const QString& source);
    QMessageBox::StandardButton checkOverwrite(const QString& dest);
    void setEntryRoot();
    void refreshEntry(const QList<QUrl>& urlList, bool remove = false);

    QString organization() const { return tr("KO Myung-Hun"); }
    QString title() const { return tr("K File Wizard"); }

    void saveSettings();
    void loadSettings();
};

#endif // KFILEWIZARD_H
