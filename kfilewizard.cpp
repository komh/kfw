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

#include <QtGui>

#include "kfilewizard.h"
#include "ui_kfilewizard.h"

#include "filesystemsortfilterproxymodel.h"
#include "entryview/entrylistmodel.h"
#include "entryview/entrytreeview.h"
#include "entryview/entrylistdelegate.h"
#include "fileoperation/fileoperation.h"
#include "fileoperation/copyfileworker.h"
#include "fileoperation/removefileworker.h"
#include "fileoperation/movefileworker.h"

#include "delayedmessagebox.h"
#include "pathcomp.h"
#include "fileiconprovider.h"

KFileWizard::KFileWizard(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KFileWizard),
    dirModel(0), dirProxyModel(0), entryModel(0), entryProxyModel(0),
    delayedMsgBox(0)
{
    ui->setupUi(this);

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(appFocusChanged(QWidget*, QWidget*)));

    initMenu();

    initLocationLine();

    initSplitter();

    currentDir = QDir::current();

    initDirTree();

    initEntryTree();

    loadSettings();
}

KFileWizard::~KFileWizard()
{
    delete ui;
}

bool KFileWizard::eventFilter(QObject* target, QEvent *event)
{
    if (target == ui->locationLine)
    {
        // the event order
        // 1. an user clicks
        // 2. FocusIn event occurs
        // 3. MouseButtonPress event ocuurs
        if (event->type() == QEvent::FocusIn)
        {
            QFocusEvent* focusEvent = reinterpret_cast<QFocusEvent*>(event);

            // so, post FocusIn event with TabFocusReason to select all
            if (focusEvent->reason() == Qt::MouseFocusReason)
                QApplication::postEvent(
                            ui->locationLine,
                            new QFocusEvent(QEvent::FocusIn,
                                            Qt::TabFocusReason));
        }
    }

    return QMainWindow::eventFilter(target, event);
}

void KFileWizard::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QMainWindow::closeEvent(event);
}

void KFileWizard::initMenu()
{
    connect(ui->about, SIGNAL(triggered()), this, SLOT(about()));
}

void KFileWizard::initLocationLine()
{
    connect(ui->locationLine, SIGNAL(returnPressed()),
            this, SLOT(locationReturnPressed()));

    ui->locationLine->installEventFilter(this);
}

void KFileWizard::initSplitter()
{
    // provide more space to a right widget, EntryTreeView
    ui->splitter->setStretchFactor(1, 1);
}

void KFileWizard::initDirTree()
{
    dirModel = new QFileSystemModel;
    dirModel->setIconProvider(new FileIconProvider);
    dirModel->setRootPath(currentDir.path());
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);

    connect(dirModel, SIGNAL(directoryLoaded(QString)),
            this, SLOT(dirLoaded(QString)));

    dirProxyModel = new FileSystemSortFilterProxyModel;
    dirProxyModel->setSourceModel(dirModel);
    dirProxyModel->setDynamicSortFilter(true);

    connect(ui->dirTree, SIGNAL(activated(QModelIndex)),
            this, SLOT(dirActivated(QModelIndex)));

    connect(ui->dirTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(dirActivated(QModelIndex)));

    ui->dirTree->setSortingEnabled(true);
    ui->dirTree->sortByColumn(0, Qt::AscendingOrder);
    ui->dirTree->setHeaderHidden(true);
    ui->dirTree->setModel(dirProxyModel);

    for (int col = 1; col < dirProxyModel->columnCount(); ++col)
         ui->dirTree->hideColumn(col);

    ui->dirTree->setTextElideMode(Qt::ElideNone);
    ui->dirTree->setEditTriggers(QAbstractItemView::SelectedClicked);
}

void KFileWizard::initEntryTree()
{
    entryModel = new EntryListModel;

    initEntryModel();

    entryProxyModel = new FileSystemSortFilterProxyModel;
    entryProxyModel->setSourceModel(entryModel);
    entryProxyModel->setDynamicSortFilter(true);

    ui->entryTree->setSortingEnabled(true);
    ui->entryTree->sortByColumn(0, Qt::AscendingOrder);
    ui->entryTree->setModel(entryProxyModel);
    ui->entryTree->setEditTriggers(QAbstractItemView::SelectedClicked);
    ui->entryTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->entryTree->setAllColumnsShowFocus(true);
    ui->entryTree->setRootIsDecorated(false);
    ui->entryTree->setItemDelegate(new EntryListDelegate);

    connect(ui->entryTree, SIGNAL(activated(QModelIndex)),
            this, SLOT(entryActivated(QModelIndex)));

    connect(ui->entryTree, SIGNAL(cdUp(QModelIndex)),
            this, SLOT(entryCdUp(QModelIndex)));
    connect(ui->entryTree, SIGNAL(paste(QList<QUrl>, bool)),
            this, SLOT(entryPaste(QList<QUrl>, bool)));
    connect(ui->entryTree, SIGNAL(remove(QList<QUrl>)),
            this, SLOT(entryRemove(QList<QUrl>)));
    connect(ui->entryTree, SIGNAL(refresh()), this, SLOT(entryRefresh()));


    setEntryRoot();
}

void KFileWizard::initEntryModel()
{
    entryModel->setIconProvider(new FileIconProvider);
    entryModel->setRootPath(currentDir.path());
    entryModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

    connect(entryModel, SIGNAL(renameBegin(QString,QString)),
            this, SLOT(renameBegin(QString,QString)));

    connect(entryModel, SIGNAL(renameEnd()), this, SLOT(renameEnd()));
}

void KFileWizard::appFocusChanged(QWidget* old, QWidget* now)
{
    if (old == ui->locationLine
            && (now == ui->dirTree || now == ui->entryTree))
        setLocationText(currentDir.path());
}

void KFileWizard::dirLoaded(const QString& dir)
{
    Q_UNUSED(dir);

    QModelIndex current =
            dirProxyModel->mapFromSource(dirModel->index(currentDir.path()));

    dirActivated(current);

    ui->dirTree->scrollTo(current);
    ui->dirTree->setCurrentIndex(current);
}

void KFileWizard::dirActivated(const QModelIndex &index)
{
    setLocationText(dirModel->filePath(dirProxyModel->mapToSource(index)));
}

void KFileWizard::entryActivated(const QModelIndex &index)
{
    QModelIndex idx(entryProxyModel->mapToSource(index));
    if (entryModel->isDir(idx))
        setLocationText(entryModel->filePath(idx));
}

void KFileWizard::entryCdUp(const QModelIndex& index)
{
    QModelIndex parentIndex(entryModel->parent(
                                entryProxyModel->mapToSource(index)));
    QString parentPath(entryModel->filePath(parentIndex));

    if (parentIndex.isValid() && parentPath != "ftp:")
        setLocationText(parentPath);
}

void KFileWizard::entryPaste(const QList<QUrl>& urlList, bool copy)
{
    QList<QUrl> urlListToSelect;

    QProgressDialog progress(this);
    progress.setWindowTitle(title());
    progress.setLabelText(copy ? tr("Preparing for copying files..."):
                                 tr("Preparing for moving files..."));
    progress.setRange(0, 100);
    progress.setModal(true);
    progress.setAutoClose(false);
    progress.setAutoReset(false);
    progress.setMinimumDuration(500);

    ui->entryTree->setUpdatesEnabled(false);

    foreach(QUrl url, urlList)
    {
        progress.setValue(0);

        if (progress.wasCanceled())
            break;

        QString source(PathComp::fixUrl(url.toString()));
        QString dest(PathComp::fixUrl(
                         currentDir.absoluteFilePath(
                             PathComp(url.path()).fileName())));

        QString canonicalSource(canonicalize(source));
        QString canonicalDest(canonicalize(dest));

        if (canonicalSource == canonicalDest)
        {
            if (copy)
            {
                dest = getNameOfCopy(source);
                canonicalDest = canonicalize(dest);
            }
            else
            {
                progress.show();

                critical(tr("The target filename is "
                            "the same as the source filename.\n\n"
                            "%1").arg(canonicalDest));
                continue;
            }
        }

        progress.setLabelText((copy ? tr("Copying %1 of %2\n\n"
                                         "%3\n\n"
                                         "to\n\n"
                                         "%4") :
                                      tr("Moving %1 of %2\n\n"
                                         "%3\n\n"
                                         "to\n\n"
                                         "%4"))
                              .arg(urlList.indexOf(url) + 1)
                              .arg(urlList.size())
                              .arg(canonicalSource)
                              .arg(canonicalDest));

        QMessageBox::StandardButton answer = checkOverwrite(&progress, dest);

        if (answer == QMessageBox::Cancel)
            break;

        if (answer == QMessageBox::No)
            continue;

        AbstractFileWorker* worker =
                copy ? qobject_cast<AbstractFileWorker*>
                            (new CopyFileWorker(source, dest)) :
                       qobject_cast<AbstractFileWorker*>
                            (new MoveFileWorker(source, dest));

        if (!fileWorker(worker, &progress))
        {
            progress.show();

            critical((copy ? tr("Failed to copy\n\n"
                                "%1\n\n"
                                "to\n\n"
                                "%2") :
                             tr("Failed to move\n\n"
                                "%1\n\n"
                                "to\n\n"
                                "%2"))
                     .arg(canonicalSource)
                     .arg(canonicalDest));
        }
        else
            urlListToSelect.append(dest);
    }

    ui->entryTree->setUpdatesEnabled(true);

    progress.close();

    // QFileSystemModel is not refreshed if only a size is changed,
    // so there is need to force it to refresh.
    refreshEntry(urlListToSelect);
}

QString KFileWizard::getNameOfCopy(const QString& source)
{
    QString result;

    int lastDot = source.lastIndexOf(".");

    if (lastDot == -1)
        lastDot = source.length();

    QString pathBaseName(source.mid(0, lastDot));
    QString suffix(source.mid(lastDot + 1 ));

    for (int i = 1;; ++i)
    {
        result = pathBaseName;
        result.append(tr(" - Copy"));
        if (i > 1)
            result.append(QString(" (%1)").arg(i));
        if (!suffix.isEmpty())
            result.append(".");
        result.append(suffix);

        if (!QFile(result).exists())
            break;
    }

    return result;
}

QMessageBox::StandardButton KFileWizard::checkOverwrite(
        QProgressDialog* progress, const QString& dest)
{
    if (QFile(dest).exists())
    {
        progress->show();

        if (dest.startsWith("ftp://"))
        {
            critical(tr("%1\n\n"
                        "This file already exists. "
                        "Overwriting is not supported by FTP.")
                            .arg(canonicalize(dest)),
                     QMessageBox::Ok);

            return QMessageBox::No;
        }

        return  question(tr("%1\n\n"
                           "This file already exists. "
                           "Are you sure to overwrite it?")
                            .arg(canonicalize(dest)),
                           QMessageBox::Yes | QMessageBox::No |
                            QMessageBox::Cancel);
    }

    return QMessageBox::Yes;
}

void KFileWizard::entryRemove(const QList<QUrl>& urlList)
{
    QString msg(urlList.size() == 1 ?
                    tr("Are you sure to delete this file?\n\n"
                       "%1")
                       .arg(canonicalize(urlList.first().toString())) :
                    tr("Are you sure to delete these %1 entries?")
                       .arg(urlList.size()));


    if (question(msg, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    QProgressDialog progress(this);
    progress.setWindowTitle(title());
    progress.setLabelText(tr("Preparing for deleting files..."));
    progress.setRange(0, urlList.size());
    progress.setModal(true);
    progress.setMinimumDuration(500);

    ui->entryTree->setUpdatesEnabled(false);

    foreach(QUrl url, urlList)
    {
        if (progress.wasCanceled())
            break;

        QString source(PathComp::fixUrl(url.toString()));

        QString canonicalSource(canonicalize(source));

        progress.setLabelText(tr("Deleting %1 of %2\n\n"
                                 "%3\n\n")
                              .arg(urlList.indexOf(url) + 1)
                              .arg(urlList.size())
                              .arg(canonicalSource));

        if (!fileWorker(new RemoveFileWorker(source), &progress))
        {
            progress.show();

            critical(tr("Failed to delete\n\n"
                        "%1")
                        .arg(canonicalSource));
        }

        progress.setValue(urlList.indexOf(url) + 1);
    }

    ui->entryTree->setUpdatesEnabled(true);

    // Refresh entry only in case of FTP
    // QFileSystemModel works fine with a local remove operation
    if (urlList.first().scheme() == "ftp")
        refreshEntry(urlList, true);
}

bool KFileWizard::fileWorker(AbstractFileWorker* worker,
                             QProgressDialog *progress)
{
    QEventLoop loop;

    QThread workerThread;

    worker->moveToThread(&workerThread);

    connect(&workerThread, SIGNAL(started()), worker, SLOT(perform()));
    connect(&workerThread, SIGNAL(finished()), &loop, SLOT(quit()),
            Qt::QueuedConnection);
    connect(progress, SIGNAL(canceled()), &loop, SLOT(quit()),
            Qt::QueuedConnection);
    connect(worker, SIGNAL(valueChanged(int)), progress, SLOT(setValue(int)));

    workerThread.start();

    loop.exec();

    if (progress->wasCanceled())
    {
        progress->show();

        QMessageBox msgBox(this);

        msgBox.setWindowTitle(title());
        msgBox.setText(tr("Canceling file operations, please wait..."));
        msgBox.setStandardButtons(QMessageBox::NoButton);

        connect(&workerThread, SIGNAL(finished()), &msgBox, SLOT(accept()));

        worker->cancel();

        msgBox.exec();
    }

    workerThread.wait();

    bool result = worker->result();

    delete worker;

    return result;
}

void KFileWizard::entryRefresh()
{
    refreshEntry(QList<QUrl>(), false);
}

void KFileWizard::renameBegin(const QString& oldName, const QString& newName)
{
    delayedMsgBox = new DelayedMessageBox(this);

    delayedMsgBox->setWindowTitle(title());
    delayedMsgBox->setText(tr("Renaming\n\n"
                              "%1\n\n"
                              "to\n\n"
                              "%2")
                              .arg(PathComp(oldName).fileName()).arg(newName));
    delayedMsgBox->trigger();
}

void KFileWizard::renameEnd()
{
    delete delayedMsgBox;
}

void KFileWizard::setLocationText(const QString& text)
{
    QString canonicalPath = canonicalize(text);

    if (canonicalPath != ui->locationLine->text())
    {
        ui->locationLine->setText(canonicalPath);

        locationReturnPressed(false);
    }
}

QString KFileWizard::canonicalize(const QString& path)
{
    if (path.indexOf(':') > 1)
        return PathComp::fixUrl(QDir::fromNativeSeparators(path));

    QString nativePath = QDir::toNativeSeparators(path);

    nativePath[0] = nativePath[0].toUpper();

    return nativePath;
}

QModelIndex KFileWizard::findDirIndex(const QString& dir)
{
    DelayedMessageBox msgBox(this);
    msgBox.setWindowTitle(title());
    msgBox.setText(tr("Checking the accessability, please wait...\n\n%1")
                        .arg(canonicalize(dir)));
    // just trigger a single shot timer without setting quit signal
    msgBox.trigger();

    QFuture<bool> future = QtConcurrent::run(QDir(dir), &QDir::isReadable);

    // don't enter into our event loop started by msgBox.exec()
    // It causes unpredictable problems. I don't know why. Just
    // guess QtConcurrent::run() conflics with a local event loop.
    // In addition, the strange is that GUI thread is not freezed
    // in spite of QFuture::result() blocking it. Magic ? ^^

    // QFuture::result() blocks until a result is ready
    if (future.result())
        return dirProxyModel->mapFromSource(dirModel->index(dir));

    return QModelIndex();
}

void KFileWizard::locationReturnPressed(bool moveFocusToEntryView)
{
    QModelIndex current(findDirIndex(ui->locationLine->text()));

    if (current.isValid())
    {
        // To canonicalize a path
        setLocationText(ui->locationLine->text());

        // already processed ?
        if (ui->locationLine->text() == canonicalize(currentDir.path()))
            return;

        currentDir.setPath(ui->locationLine->text());
        currentDir.makeAbsolute();

        setEntryRoot();

        if (moveFocusToEntryView)
            ui->entryTree->setFocus();
    }
    else
    {
        critical(tr("This is not a directory or not accessible.\n\n%1")
                    .arg(ui->locationLine->text()));

        ui->locationLine->selectAll();
    }
}

void KFileWizard::setEntryRoot()
{
    // already fetched ?
    if (PathComp::fixUrl(entryModel->rootPath()) != currentDir.path())
    {
        DelayedMessageBox msgBox(this);

        msgBox.setWindowTitle(title());
        msgBox.setText(tr("Reading directory entries, please wait...\n\n%1")
                            .arg(canonicalize(currentDir.path())));

        msgBox.setQuitSignal(entryModel, SIGNAL(directoryLoaded(QString)));
        msgBox.trigger();

        entryModel->setRootPath(currentDir.path());

        msgBox.exec();
    }

    QModelIndex rootIndex = entryModel->index(currentDir.path());
    entryModel->setRootIndex(rootIndex);

    rootIndex = entryProxyModel->mapFromSource(rootIndex);
    ui->entryTree->setRootIndex(rootIndex);

    // select a corresponding entry of dir tree
    QModelIndex current =
            dirProxyModel->mapFromSource(dirModel->index(currentDir.path()));

    ui->dirTree->setCurrentIndex(current);
    ui->dirTree->scrollTo(current);
}

void KFileWizard::refreshEntry(const QList<QUrl>& urlList, bool remove)
{
    ui->entryTree->setUpdatesEnabled(false);

    QString newPath;

    // removed ?
    if (remove)
    {
        // This is only for FTP. QFileSystemWatcher does not work with it

        QModelIndex parent = entryProxyModel->mapFromSource(
                                    entryModel->index(currentDir.path()));

        // Select a previous row like Qt does
        int row = ui->entryTree->currentIndex().row();
        while (row >= 0)
        {
            if (!urlList.contains(
                            entryModel->filePath(
                                entryProxyModel->mapToSource(
                                    entryProxyModel->index(row, 0, parent)))))
                break;

            --row;
        }

        // reached to the top ?
        if (row < 0)
        {
            // then try a next row
            row = ui->entryTree->currentIndex().row();
            while (row < entryProxyModel->rowCount(parent))
            {
                if (!urlList.contains(
                                entryModel->filePath(
                                    entryProxyModel->mapToSource(
                                        entryProxyModel->index(row, 0,
                                                               parent)))))
                    break;

                ++row;
            }

            if (row == entryProxyModel->rowCount(parent))
                row = 0;    // all removed
        }

        newPath = entryModel->filePath(
                    entryProxyModel->mapToSource(
                        entryProxyModel->index(row, 0, parent)));
    }

    QByteArray headerState(ui->entryTree->header()->saveState());

    entryProxyModel->setSourceModel(0);
    delete entryModel;

    entryModel = new EntryListModel;

    DelayedMessageBox msgBox(this);

    msgBox.setWindowTitle(title());
    msgBox.setText(tr("Refreshing directory entries, please wait...\n\n%1")
                        .arg(currentDir.path()));

    msgBox.setQuitSignal(entryModel, SIGNAL(directoryLoaded(QString)));
    msgBox.trigger();

    // signal to FtpFileEngine to refresh entries
    if (currentDir.path().startsWith("ftp:"))
    {
        // QDir::filePath() does not work with ":refresh:"
        QFile(PathComp::merge(currentDir, ":refresh:")).exists();
    }

    initEntryModel();

    // wait for directory to be loaded
    msgBox.exec();

    entryProxyModel->setSourceModel(entryModel);
    ui->entryTree->setModel(entryProxyModel);

    setEntryRoot();

    ui->entryTree->header()->restoreState(headerState);

    // select proper entries

    QItemSelectionModel* selection = ui->entryTree->selectionModel();

    QModelIndex newCurrent;

    if (remove)
        newCurrent = entryProxyModel->mapFromSource(
                            entryModel->index(newPath));
    else
    {
        // select the given list

        foreach (QUrl url, urlList)
        {
            QModelIndex index(entryProxyModel->mapFromSource(
                                  entryModel->index(url.toString())));

            selection->select(index, QItemSelectionModel::Select |
                                     QItemSelectionModel::Rows);

            // find a bottom entry
            if (!newCurrent.isValid() || newCurrent < index)
                newCurrent = index;
        }
    }

    // set current
    selection->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);

    ui->entryTree->scrollTo(newCurrent);

    ui->entryTree->setUpdatesEnabled(true);
}

void KFileWizard::saveSettings()
{
    QSettings settings(organization(), title());

    settings.beginGroup("mainwindow");
    settings.setValue("geometry", saveGeometry());

    settings.beginGroup("splitter");
    settings.setValue("state", ui->splitter->saveState());

    settings.beginGroup("entrytree");
    settings.setValue("headerstate", ui->entryTree->header()->saveState());

    settings.endGroup(); // entrytree
    settings.endGroup(); // splitter
    settings.endGroup(); // mainwindow
}

void KFileWizard::loadSettings()
{
    QSettings settings(organization(), title());

    settings.beginGroup("mainwindow");
    restoreGeometry(settings.value("geometry").toByteArray());

    settings.beginGroup("splitter");
    ui->splitter->restoreState(settings.value("state").toByteArray());

    settings.beginGroup("entrytree");
    ui->entryTree->header()->restoreState(
                settings.value("headerstate").toByteArray());

    settings.endGroup(); // entrytree
    settings.endGroup(); // splitter
    settings.endGroup(); // mainwindow
}

void KFileWizard::about()
{
    QMessageBox::about( this, tr("About K File Wizard"), tr(
"<h2>K File Wizard alpha</h2>"
"<p>Copyright &copy; 2014 by KO Myung-Hun "
"<a href=mailto:komh@chollian.net>&lt;komh@chollian.net&gt;</a>"
"<p>K File Wizard is a program to provide the integreated file management "
"both for the local files and for the remote files."
"<p>If you want to promote to develop this program, then donate at the below "
"web page, please."
"<p align=center><a href=http://www.ecomstation.co.kr/komh/donate.html>"
"http://www.ecomstation.co.kr/komh/donate.html</a>"
"<p>This program comes with ABSOLUTELY NO WARRANTY. This is free software, "
"and you are welcome to redistribute it under certain conditions. See "
"<a href=http://www.gnu.org/licenses/gpl.html>the GPL v3 license</a> "
"for details."
                      ));
}
