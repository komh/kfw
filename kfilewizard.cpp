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

#include "locationcompleter.h"

#include "connecttodialog.h"
#include "addressbookdialog.h"

// true : created
// false : attached or failed
static bool createSharedMem(SharedMemory* mem)
{
    if (mem->attach())
        return false;

    return mem->create(sizeof(quint16) * 4);
}

static QPoint posFromSharedMem(SharedMemory* mem)
{
    if (!mem->lock())
        return QPoint();

    const quint16* data = static_cast<const quint16*>(mem->data());

    QPoint point(data[0], data[1]);

    mem->unlock();

    return point;
}

static void storePosToSharedMem(SharedMemory* mem, int x, int y)
{
    if (!mem->lock())
        return;

    quint16* data = static_cast<quint16*>(mem->data());

    data[0] = x;
    data[1] = y;

    mem->unlock();
}

static QSize sizeFromSharedMem(SharedMemory* mem)
{
    if (!mem->lock())
        return QSize();

    const quint16* data = static_cast<const quint16*>(mem->data());

    QSize size(data[2], data[3]);

    mem->unlock();

    return size;
}

static void storeSizeToSharedMem(SharedMemory* mem, int w, int h)
{
    if (!mem->lock())
        return;

    quint16* data = static_cast<quint16*>(mem->data());

    data[2] = w;
    data[3] = h;

    mem->unlock();
}

// nameListToAdd: a list of entry names to add
static QStringList entryList(const QString& dir,
                             const QStringList& nameListToAdd,
                             bool parentFirst, const QProgressDialog* progress)
{
    if (nameListToAdd.isEmpty())
        return QStringList();

    QStringList result;

    QStringListIterator it(nameListToAdd);

    while (!progress->wasCanceled() && it.hasNext())
    {
        QFileInfo info(PathComp::merge(dir, it.next()));

        if (parentFirst)
            result << PathComp::fixUrl(info.filePath());

        if (info.isDir())
        {

            QDir dir(info.filePath());

            result << entryList(info.filePath(),
                                dir.entryList(QDir::AllEntries |
                                              QDir::NoDotAndDotDot),
                                parentFirst, progress);

        }

        if (!parentFirst)
            result << PathComp::fixUrl(info.filePath());
    }

    return result;
}

// urlListToAdd : a list of fully qualified paths to add
static QStringList entryList(const QList<QUrl>& urlListToAdd,
                             bool parentFirst, const QProgressDialog* progress)
{
    if (urlListToAdd.isEmpty())
        return QStringList();

    QString dir(PathComp(urlListToAdd.first().toString()).dir());
    QStringList nameList;

    foreach (QUrl url, urlListToAdd)
        nameList << PathComp(url.toString()).fileName();

    return entryList(dir, nameList, parentFirst, progress);
}

static QStringList entryListWorker(const QList<QUrl>& urlListToAdd,
                                   bool parentFirst,
                                   const QProgressDialog& progress)
{
    QFutureWatcher<QStringList> watcher;
    QEventLoop loop;

    QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));

    QFuture<QStringList> future = QtConcurrent::run(entryList, urlListToAdd,
                                                    parentFirst, &progress);

    watcher.setFuture(future);

    if (future.isRunning())
        loop.exec();

    if (progress.wasCanceled())
        return QStringList();

    return future.result();
}

static void refreshFtpDirList(const QStringList& ftpDirList)
{
    QStringListIterator it(ftpDirList);

    while (it.hasNext())
    {
        QString dir(it.next());

        // signal to FtpFileEngine to refresh entries
        if (PathComp::isFtpPath(dir))
        {
            // QDir::filePath() does not work with ":refresh:"
            QFile(PathComp::merge(dir, ":refresh:")).exists();
        }
    }
}

static void refreshFtpDir(const QString& ftpDir)
{
    refreshFtpDirList(QStringList() << ftpDir);
}

KFileWizard::KFileWizard(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KFileWizard),
    dirModel(0), dirProxyModel(0), entryModel(0), entryProxyModel(0),
    delayedMsgBox(this), sharedMem(title())
{
    ui->setupUi(this);

    // global settings for QSettings
    QApplication::setOrganizationName(organization());
    QApplication::setApplicationName(title());

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(appFocusChanged(QWidget*, QWidget*)));

    initMenu();

    initLocationLine();

    initSplitter();

    currentDir = QDir::current();

    initDirTree();

    initEntryTree();

    loadSettings();

    // save initial pos and initial size to refer to them later
    initialPos = pos();
    initialSize = size();

    // move out of screen to prevent a main window from flickering
    // until it is located properly later
    move(-50000, -50000);
}

KFileWizard::~KFileWizard()
{
#ifdef USE_FTP_CONNECTION_CACHE
    // close all the FTP connections
    QFile("ftp:///:closeall:").exists();
#endif

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

void KFileWizard::lazyInitGeometry()
{
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

bool KFileWizard::event(QEvent *event)
{
    // lazyInitGeometry() event ?
    if (event->type() == QEvent::User)
    {
        // first instance ?
        if (createSharedMem(&sharedMem))
        {
            storePosToSharedMem(&sharedMem, initialPos.x(), initialPos.y());
            storeSizeToSharedMem(&sharedMem, initialSize.width(),
                                 initialSize.height());

            move(initialPos);
        }
        else // second instance
        {
            QPoint pos(posFromSharedMem(&sharedMem));
            QSize size(sizeFromSharedMem(&sharedMem));

            // calculate thickness of a left frame
            int leftFrameSize = geometry().x() - frameGeometry().x();

            // calculate thickness of a top frame including a title bar
            int topFrameSize = geometry().y() - frameGeometry().y();

            // calculate thickness of both a left frame and a right frame
            int horizontalFrameSize =
                    frameGeometry().width() - geometry().width();

            // calculate thickness of both a top frame and a bottom frame
            int verticalFrameSize =
                    frameGeometry().height() - geometry().height();

            // move by leftFrameSize * 5 and topFrameSize
            pos.setX(pos.x() + leftFrameSize * 5);
            pos.setY(pos.y() + topFrameSize);

            if (pos.x() + size.width() + horizontalFrameSize
                    >= QApplication::desktop()->width())
                pos.setX(0);

            if (pos.y() + size.height() + verticalFrameSize
                    >= QApplication::desktop()->height())
                pos.setY(0);

            move(pos);

            if (!size.isNull())
                resize(size);
        }

        return true;
    }

    return QMainWindow::event(event);
}

void KFileWizard::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QMainWindow::closeEvent(event);
}

void KFileWizard::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);

    storePosToSharedMem(&sharedMem, frameGeometry().x(), frameGeometry().y());
}

void KFileWizard::resizeEvent(QResizeEvent *event)
{
    storeSizeToSharedMem(&sharedMem, event->size().width(),
                         event->size().height());
}

void KFileWizard::initMenu()
{
    connect(ui->about, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->fileConnectTo, SIGNAL(triggered()), this, SLOT(connectTo()));
    connect(ui->fileOpenAddressBook, SIGNAL(triggered()),
            this, SLOT(openAddressBook()));
}

void KFileWizard::initLocationLine()
{
    connect(ui->locationLine, SIGNAL(returnPressed()),
            this, SLOT(locationReturnPressed()));

    ui->locationLine->installEventFilter(this);

    QCompleter* completer = new LocationCompleter(this);

    locationCompleterModel = new QFileSystemModel(this);

    locationCompleterModel->setIconProvider(new FileIconProvider);
    locationCompleterModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    locationCompleterModel->setRootPath("");

    completer->setModel(locationCompleterModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    ui->locationLine->setCompleter(completer);
}

void KFileWizard::initSplitter()
{
    // provide more space to a right widget, EntryTreeView
    ui->splitter->setStretchFactor(1, 1);
}

void KFileWizard::initDirTree()
{
    dirModel = new EntryListModel(false);
    dirModel->setIconProvider(new FileIconProvider);
    dirModel->setRootPath(currentDir.path());
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);

    connect(dirModel, SIGNAL(directoryLoaded(QString)),
            this, SLOT(dirLoaded(QString)));
    connect(dirModel, SIGNAL(renameBegin(QString,QString)),
            this, SLOT(renameBegin(QString,QString)));
    connect(dirModel, SIGNAL(renameEnd(bool)), this, SLOT(renameEnd(bool)));

    dirProxyModel = new FileSystemSortFilterProxyModel;
    dirProxyModel->setSourceModel(dirModel);
    dirProxyModel->setDynamicSortFilter(true);

    connect(ui->dirTree, SIGNAL(activated(QModelIndex)),
            this, SLOT(dirActivated(QModelIndex)));

    connect(ui->dirTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(dirActivated(QModelIndex)));

    connect(ui->dirTree, SIGNAL(dropped(QList<QUrl>,QString,bool)),
            this, SLOT(dirDropped(QList<QUrl>,QString,bool)));

    connect(ui->dirTree, SIGNAL(paste(QList<QUrl>, bool)),
            this, SLOT(entryPaste(QList<QUrl>, bool)));

    connect(ui->dirTree, SIGNAL(remove(QList<QUrl>)),
            this, SLOT(entryRemove(QList<QUrl>)));

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
    connect(ui->entryTree, SIGNAL(dropped(QList<QUrl>,QString,bool)),
            this, SLOT(dirDropped(QList<QUrl>,QString,bool)));

    setEntryRoot();
}

void KFileWizard::initEntryModel()
{
    entryModel->setIconProvider(new FileIconProvider);
    entryModel->setRootPath(currentDir.path());
    entryModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

    connect(entryModel, SIGNAL(renameBegin(QString,QString)),
            this, SLOT(renameBegin(QString,QString)));

    connect(entryModel, SIGNAL(renameEnd(bool)), this, SLOT(renameEnd(bool)));
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

    // direcoryLoaded() signals occurs in this order if rootPath is
    // x:/path/to/dir
    // 1. x:/path/to/dir
    // 2. x:/
    // 3. x:/path
    // 4. x:/path/to
    // so disconnect directoryLoaded() signal if dir is the parent of
    // currentDir
    if (dir == PathComp(currentDir.absolutePath()).dir())
        disconnect(dirModel, SIGNAL(directoryLoaded(QString)), this, 0);
}

void KFileWizard::dirActivated(const QModelIndex &index)
{
    setLocationText(dirModel->filePath(dirProxyModel->mapToSource(index)));
}

void KFileWizard::dirDropped(const QList<QUrl> &urlList, const QString &to, bool copy)
{
    copyUrlsTo(urlList, to, copy);
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

    if (index.isValid())
        setLocationText(parentPath);
}

void KFileWizard::entryPaste(const QList<QUrl>& urlList, bool copy)
{
    copyUrlsTo(urlList, currentDir.path(), copy);
}

void KFileWizard::copyUrlsTo(const QList<QUrl> &urlList, const QString &to,
                             bool copy)
{
    // to a drive list or to a ftp list ? No...
    if (PathComp(to).isDriveList())
        return;

    QProgressDialog progress(this);
    progress.setWindowTitle(title());
    progress.setLabelText(copy ? tr("Preparing for copying files..."):
                                 tr("Preparing for moving files..."));
    progress.setRange(0, 100);
    progress.setModal(true);
    progress.setAutoClose(false);
    progress.setAutoReset(false);
    progress.setMinimumDuration(500);
    progress.setValue(0);

    QStringList sourceListToCopy(entryListWorker(urlList, true, progress));

    if (progress.wasCanceled())
        return;

    QString sourceDir(PathComp(sourceListToCopy.first()).dir());

    QString dirName;
    QString dirNameCopy;

    QString skippedDir;

    QStringList dirListToRemove;
    QStringList dirListToRefresh;

    QList<QUrl> urlListToSelect;

    ui->entryTree->setUpdatesEnabled(false);

    foreach(QString source, sourceListToCopy)
    {
        if (progress.wasCanceled())
            break;

        if (!skippedDir.isEmpty() && source.startsWith(skippedDir))
            continue;

        progress.setValue(0);

        bool sourceIsDir = QFileInfo(source).isDir();
        bool sourceIsInSourceDir = PathComp(source).dir() == sourceDir;

        QString dest(PathComp::merge(to, source.mid(sourceDir.length())));

        if (sourceIsInSourceDir && sourceIsDir)
            dirNameCopy = dirName = dest;

        if (dirName != dirNameCopy)
            dest.replace(dirName, dirNameCopy);

        QString canonicalSource(PathComp(source).canonicalPath());
        QString canonicalDest(PathComp(dest).canonicalPath());

        if (canonicalSource == canonicalDest)
        {
            if (copy)
            {
                dest = getNameOfCopy(source);
                canonicalDest = PathComp(dest).canonicalPath();
            }
            else
            {
                progress.show();

                critical(tr("The target filename is "
                            "the same as the source filename.\n\n"
                            "%1").arg(canonicalDest));

                if (sourceIsDir)
                {
                    skippedDir = source;

                    PathComp::addDirSeparator(skippedDir);
                }

                continue;
            }
        }

        if (sourceIsDir && PathComp(source).isParentDirOf(dest))
        {
            progress.show();

            critical(tr("Source is a parent directory of target.\n\n"
                        "Source: %1\n\n"
                        "Target: %2")
                     .arg(canonicalSource).arg(canonicalDest));

            skippedDir = source;

            PathComp::addDirSeparator(skippedDir);

            continue;
        }

        progress.setLabelText((copy ? tr("Copying %1 of %2\n\n"
                                         "%3\n\n"
                                         "to\n\n"
                                         "%4") :
                                      tr("Moving %1 of %2\n\n"
                                         "%3\n\n"
                                         "to\n\n"
                                         "%4"))
                              .arg(sourceListToCopy.indexOf(source) + 1)
                              .arg(sourceListToCopy.size())
                              .arg(canonicalSource)
                              .arg(canonicalDest));

        QMessageBox::StandardButton answer = checkOverwrite(&progress, dest);

        if (answer == QMessageBox::Cancel)
        {
            progress.cancel();
            break;
        }

        if (answer == QMessageBox::No)
        {
            if (sourceIsDir)
            {
                skippedDir = source;

                PathComp::addDirSeparator(skippedDir);
            }

            continue;
        }

        AbstractFileWorker* worker =
                copy || sourceIsDir ?
                    qobject_cast<AbstractFileWorker*>
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

            if (sourceIsDir)
            {
                skippedDir = source;

                PathComp::addDirSeparator(skippedDir);
            }
        }
        else
        {
            if (!copy && sourceIsDir)
                dirListToRemove.append(source);

            if (sourceIsInSourceDir)
            {
                urlListToSelect.append(dest);

                if (sourceIsDir)
                    dirNameCopy = dest;
            }

            if (PathComp::isFtpPath(dest))
            {
                QString destDir(PathComp(dest).dir());

                if (!dirListToRefresh.contains(destDir))
                    dirListToRefresh << destDir;
            }
        }
    }

    if (!progress.wasCanceled())
    {
        // remove directories not moved
        QStringListIterator it(dirListToRemove);

        it.toBack();
        while (it.hasPrevious())
        {
            QString dir(it.previous());

            if (!fileWorker(new RemoveFileWorker(dir), &progress))
            {
                progress.show();

                critical(tr("Failed to delete\n\n"
                            "%1")
                         .arg(PathComp(dir).canonicalPath()));
            }
        }
    }

    ui->entryTree->setUpdatesEnabled(true);

    progress.close();

    if (!dirListToRefresh.isEmpty())
    {
        DelayedMessageBox msgBox(this);

        msgBox.setWindowTitle(title());
        msgBox.setText(tr("Please wait while finalizing..."));

        msgBox.setMinimumDuration(0);
        msgBox.trigger();

        refreshFtpDirList(dirListToRefresh);
    }

    // QFileSystemModel is not refreshed if only a size is changed,
    // so there is need to force it to refresh.
    refreshEntry(urlListToSelect);
}

QString KFileWizard::getNameOfCopy(const QString& source)
{
    QUrl result(source);

    int lastDot = result.path().lastIndexOf(".");

    if (lastDot == -1)
        lastDot = result.path().length();

    QString pathBaseName(result.path().mid(0, lastDot));
    QString suffix(result.path().mid(lastDot + 1 ));

    for (int i = 1;; ++i)
    {
        QString path(pathBaseName);
        path.append(tr(" - Copy"));
        if (i > 1)
            path.append(QString(" (%1)").arg(i));
        if (!suffix.isEmpty())
            path.append(".");
        path.append(suffix);

        result.setPath(path);
        if (!QFile(result.toString()).exists())
            break;
    }

    return result.toString();
}

QMessageBox::StandardButton KFileWizard::checkOverwrite(
        QProgressDialog* progress, const QString& dest)
{
    if (QFile(dest).exists())
    {
        progress->show();

        if (PathComp::isFtpPath(dest))
        {
            critical(tr("%1\n\n"
                        "This file already exists. "
                        "Overwriting is not supported by FTP.")
                            .arg(PathComp(dest).canonicalPath()),
                     QMessageBox::Ok);

            return QMessageBox::No;
        }

        return  question(tr("%1\n\n"
                           "This file already exists. "
                           "Are you sure to overwrite it?")
                            .arg(PathComp(dest).canonicalPath()),
                           QMessageBox::Yes | QMessageBox::No |
                            QMessageBox::Cancel);
    }

    return QMessageBox::Yes;
}

void KFileWizard::entryRemove(const QList<QUrl>& urlList)
{
    removeUrls(urlList);
}

void KFileWizard::removeUrls(const QList<QUrl> &urlList)
{
    // root directory ?
    if (PathComp(urlList.first().toString()).isRoot())
        return;

    QString msg(urlList.size() == 1 ?
                    tr("Are you sure to delete this entry?\n\n"
                       "%1")
                       .arg(PathComp(urlList.first().toString()).canonicalPath()) :
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
    progress.setValue(0);

    QStringList sourceListToRemove(entryListWorker(urlList, false, progress));

    if (progress.wasCanceled())
        return;

    progress.setRange(0, sourceListToRemove.size());

    ui->entryTree->setUpdatesEnabled(false);

    foreach(QString source, sourceListToRemove)
    {
        if (progress.wasCanceled())
            break;

        QString canonicalSource(PathComp(source).canonicalPath());

        progress.setLabelText(tr("Deleting %1 of %2\n\n"
                                 "%3\n\n")
                              .arg(sourceListToRemove.indexOf(source) + 1)
                              .arg(sourceListToRemove.size())
                              .arg(canonicalSource));

        if (!fileWorker(new RemoveFileWorker(source), &progress))
        {
            progress.show();

            critical(tr("Failed to delete\n\n"
                        "%1")
                        .arg(canonicalSource));
        }

        progress.setValue(sourceListToRemove.indexOf(source) + 1);
    }

    ui->entryTree->setUpdatesEnabled(true);

    // Refresh entry only in case of FTP
    // QFileSystemModel works fine with a local remove operation
    if (1 || PathComp::isFtpPath(urlList.first().toString()))
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

    if (workerThread.isRunning())
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

        if (workerThread.isRunning())
            msgBox.exec();
    }

    workerThread.wait();

    bool result = worker->result();

    delete worker;

    return result;
}

void KFileWizard::entryRefresh()
{
    refreshEntry(QList<QUrl>(), false, true);
}

void KFileWizard::renameBegin(const QString& oldName, const QString& newName)
{
    if (sender() == dirModel)
    {
        dirModel->setRootPath("");
        entryModel->setRootPath("");
        locationCompleterModel->setRootPath("");
    }

    delayedMsgBox.setWindowTitle(title());
    delayedMsgBox.setText(tr("Renaming\n\n"
                             "%1\n\n"
                             "to\n\n"
                             "%2")
                          .arg(PathComp(oldName).fileName()).arg(newName));
    delayedMsgBox.trigger();
}

void KFileWizard::renameEnd(bool success)
{
    delayedMsgBox.close();

    if (success)
    {
        if (sender() == dirModel)
        {
            QMetaObject::invokeMethod(this, "dirActivated", Qt::QueuedConnection,
                                      Q_ARG(QModelIndex,
                                            ui->dirTree->currentIndex()));
        }

        QMetaObject::invokeMethod(this, "entryRefresh", Qt::QueuedConnection);
    }
}

void KFileWizard::setLocationText(const QString& text, bool focusToEntry)
{
    QString nativePath = PathComp(text).nativePath();

    if (nativePath != ui->locationLine->text())
    {
        ui->locationLine->setText(nativePath);

        locationReturnPressed(focusToEntry, false);
    }
}

QModelIndex KFileWizard::findDirIndex(const QString& dir)
{
    DelayedMessageBox msgBox(this);
    msgBox.setWindowTitle(title());
    msgBox.setText(tr("Checking the accessability, please wait...\n\n%1")
                        .arg(PathComp(dir).canonicalPath()));
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

void KFileWizard::locationReturnPressed(bool focusToEntry, bool bySignal)
{
    // cdUp() from a drive root or a valid directory entry ?
    if ((!bySignal && ui->locationLine->text().isEmpty())
            || findDirIndex(ui->locationLine->text()).isValid())
    {
        // To convert to a native path
        setLocationText(ui->locationLine->text(), focusToEntry);

        // already processed ?
        if (ui->locationLine->text()
                == PathComp(currentDir.path()).nativePath())
            return;

        currentDir.setPath(ui->locationLine->text());
        if (!currentDir.path().isEmpty())
            currentDir.makeAbsolute();

        // remove password and so on
        ui->locationLine->setText(PathComp(currentDir.path()).canonicalPath());

        setEntryRoot();

        if (focusToEntry)
            ui->entryTree->setFocus();
    }
    else
    {
        critical(tr("This is not a directory or not accessible.\n\n%1")
                    .arg(ui->locationLine->text()));

        // Select all only if a location line has a focus.
        // This prevents a location line from having a focus when cd fails
        // in an entry view
        if (ui->locationLine->hasFocus())
            ui->locationLine->selectAll();
    }
}

void KFileWizard::setEntryRoot()
{
    // already fetched ?
    if (entryModel->rootPath() != currentDir.path())
    {
        DelayedMessageBox msgBox(this);

        msgBox.setWindowTitle(title());
        msgBox.setText(tr("Reading directory entries, please wait...\n\n%1")
                            .arg(PathComp(currentDir.path()).canonicalPath()));

        msgBox.setQuitSignal(entryModel, SIGNAL(directoryLoaded(QString)));
        msgBox.trigger();


        entryModel->setRootPath(currentDir.path());

        // no need to wait for populating a drive list
        if (!PathComp(entryModel->rootPath()).isDriveList())
            msgBox.exec();
    }

    QModelIndex rootIndex = entryModel->index(currentDir.path());
    rootIndex = entryProxyModel->mapFromSource(rootIndex);
    ui->entryTree->setRootIndex(rootIndex);

    ui->entryTree->scrollToTop();

    dirModel->setRootPath("");
    dirModel->setRootPath(currentDir.path());

    // select a corresponding entry of dir tree
    QModelIndex current =
            dirProxyModel->mapFromSource(dirModel->index(currentDir.path()));

    ui->dirTree->setCurrentIndex(current);
    ui->dirTree->scrollTo(current);

    locationCompleterModel->setRootPath("");
    locationCompleterModel->setRootPath(currentDir.path());
}

QString KFileWizard::newPathForRemove(const QList<QUrl>& urlList)
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
                                entryProxyModel->index(row, 0, parent)))))
                break;

            ++row;
        }

        if (row == entryProxyModel->rowCount(parent))
            row = 0;    // all removed
    }

    return entryModel->filePath(entryProxyModel->mapToSource(
                                    entryProxyModel->index(row, 0, parent)));
}

void KFileWizard::refreshEntryModel(bool force)
{
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

    if (force)
        refreshFtpDir(currentDir.path());

    initEntryModel();

    msgBox.exec();

    // this should right before entryProxyModel->setSourceModel().
    // otherwise, a scroll bar occurs  when refreshing.
    // frankly, I don't know why. TT
    ui->entryTree->header()->restoreState(headerState);

    entryProxyModel->setSourceModel(entryModel);
    ui->entryTree->setModel(entryProxyModel);

    setEntryRoot();

    // trcik to make sure that a header is visible.
    // a header is sometimes invisible on OS/2 after refreshing, especially,
    // ftp
    ui->entryTree->setHeaderHidden(true);
    ui->entryTree->setHeaderHidden(false);
}

void KFileWizard::selectEntries(const QList<QUrl>& urlListToSelect, bool remove)
{
    // select proper entries

    QItemSelectionModel* selection = ui->entryTree->selectionModel();

    QModelIndex newCurrent;

    if (remove)
        newCurrent = entryProxyModel->mapFromSource(
                        entryModel->index(urlListToSelect.first().toString()));
    else
    {
        // select the given list

        foreach (QUrl url, urlListToSelect)
        {
            QModelIndex index(entryProxyModel->mapFromSource(
                                  entryModel->index(url.toString())));

            selection->select(index, QItemSelectionModel::Select |
                                     QItemSelectionModel::Rows);

            // in case of a different dir, need to preserve a current index
            // as well ?
            // just find a bottom entry
            if (!newCurrent.isValid() || newCurrent < index)
                newCurrent = index;
        }
    }

    // set current
    selection->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);

    ui->entryTree->scrollTo(newCurrent);
}

void KFileWizard::refreshEntry(const QList<QUrl>& urlList, bool remove,
                               bool force)
{
    // do not refresh on a drive list
    if (PathComp(currentDir.path()).isDriveList())
        return;

    static bool refreshing = false;

    // prevent nested refresh
    if (refreshing)
        return;

    refreshing = true;

    QString urlDir(urlList.isEmpty() ?
                       QString() : PathComp(urlList.first().toString()).dir());

    bool isUrlDifferentDir =
            urlDir !=  PathComp::fixUrl(currentDir.absolutePath());

    // a target dir is different from the current dir, then preserve
    // current selections, otherwise select the new targets
    QList<QUrl> urlListToSelect = isUrlDifferentDir ?
                ui->entryTree->selectedUrlList() : urlList;

    ui->entryTree->setUpdatesEnabled(false);

    // removed ?
    if (remove)
    {
        urlListToSelect.clear();
        urlListToSelect.append(newPathForRemove(urlList));
    }

    refreshEntryModel(force);

    selectEntries(urlListToSelect, remove);

    ui->entryTree->setUpdatesEnabled(true);

    refreshing = false;
}

void KFileWizard::saveSettings()
{
    QSettings settings;

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
    QSettings settings;

    settings.beginGroup("mainwindow");

    if (settings.contains("geometry"))
        restoreGeometry(settings.value("geometry").toByteArray());
    else
    {
        // no a main window geometry
        // locate a main window at center. this is a default action of Qt
        move((QApplication::desktop()->width() - frameGeometry().width()) / 2,
             (QApplication::desktop()->height() - frameGeometry().height()) / 2);
    }

    settings.beginGroup("splitter");

    if (settings.contains("state"))
        ui->splitter->restoreState(settings.value("state").toByteArray());

    settings.beginGroup("entrytree");

    if (settings.contains("headerstate"))
    {
        ui->entryTree->header()->restoreState(
                    settings.value("headerstate").toByteArray());
    }

    settings.endGroup(); // entrytree
    settings.endGroup(); // splitter
    settings.endGroup(); // mainwindow
}

void KFileWizard::about()
{
    QMessageBox::about( this, tr("About K File Wizard"), tr(
"<h2>K File Wizard %1</h2>"
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
                            ).arg(version()));
}

void KFileWizard::connectTo()
{
    ConnectToDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        // To select all when connection fails
        ui->locationLine->setFocus();

        setLocationText(dialog.locationUrl(), true);
    }
}

void KFileWizard::openAddressBook()
{
    AddressBookDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        // To select all when connection fails
        ui->locationLine->setFocus();

        setLocationText(dialog.locationUrl(), true);
    }
}
