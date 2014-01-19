#include <QtGui>

#include "kfilewizard.h"
#include "ui_kfilewizard.h"

#include "filesystemsortfilterproxymodel.h"
#include "entrylistmodel.h"
#include "entrytreeview.h"
#include "fileoperation.h"
#include "msleep.h"

KFileWizard::KFileWizard(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KFileWizard),
    locationMouseFocus(false),
    dirModel(0), dirProxyModel(0), entryModel(0), entryProxyModel(0)
{
    ui->setupUi(this);

    initLocationLine();

    initSplitter();

    currentDir = QDir::current();

    initDirTree();

    initEntryTree();
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
        switch (event->type())
        {
        case QEvent::FocusIn:
        {
            QFocusEvent* focusEvent = reinterpret_cast<QFocusEvent*>(event);

            if (focusEvent->reason() == Qt::MouseFocusReason)
                locationMouseFocus = true;

            // pass to the parent
            break;
        }

        case QEvent::MouseButtonPress:
            if (locationMouseFocus)
            {
                ui->locationLine->selectAll();

                locationMouseFocus = false;

                // consume this event
                return true;
            }

            // pass to the parent
            break;

        default:
            break;
        }
    }
    else if (event->type() == QEvent::FocusIn)  // dirTree or entryTree
        setLocationText(currentDir.path());

    return QMainWindow::eventFilter(target, event);
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
    ui->dirTree->installEventFilter(this);

    dirModel = new QFileSystemModel;
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
    ui->entryTree->installEventFilter(this);

    entryModel = new EntryListModel;
    entryModel->setRootPath(currentDir.path());
    entryModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

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

    connect(ui->entryTree, SIGNAL(activated(QModelIndex)),
            this, SLOT(entryActivated(QModelIndex)));

    connect(ui->entryTree, SIGNAL(cdUp(QModelIndex)),
            this, SLOT(entryCdUp(QModelIndex)));
    connect(ui->entryTree, SIGNAL(paste(QList<QUrl>)),
            this, SLOT(entryPaste(QList<QUrl>)));
    connect(ui->entryTree, SIGNAL(remove(QList<QUrl>)),
            this, SLOT(entryRemove(QList<QUrl>)));

    setEntryRoot();
}

void KFileWizard::dirLoaded(const QString& dir)
{
    Q_UNUSED(dir);

    QModelIndex current =
            dirProxyModel->mapFromSource(dirModel->index(currentDir.path()));

    dirActivated(current);
}

void KFileWizard::dirActivated(const QModelIndex &index)
{
    setLocationText(dirModel->filePath(dirProxyModel->mapToSource(index)),
                    true);
}

void KFileWizard::entryActivated(const QModelIndex &index)
{
    QModelIndex idx(entryProxyModel->mapToSource(index));
    if (entryModel->isDir(idx))
        setLocationText(entryModel->filePath(idx));
}

void KFileWizard::entryCdUp(const QModelIndex& index)
{
    if (entryModel->parent(entryProxyModel->mapToSource(index)).isValid())
        setLocationText(entryModel->filePath(
                            entryModel->parent(
                                entryProxyModel->mapToSource(index))));
}

void KFileWizard::entryPaste(const QList<QUrl>& urlList)
{
    QProgressDialog progress(this);
    progress.setWindowTitle(tr("K File Wizard"));
    progress.setLabelText(tr("Copying files..."));
    progress.setRange(0, 100);
    progress.setModal(true);
    progress.setAutoClose(false);
    progress.show();

    FileOperation fileOp;

    foreach(QUrl url, urlList)
    {
        if (progress.wasCanceled())
            break;

        QString source(FileOperation::fixUrl(url.toString()));
        QString dest(FileOperation::fixUrl(
                         currentDir.absoluteFilePath(
                             QFileInfo(url.path()).fileName())));

        QString canonicalSource(canonicalize(source));
        QString canonicalDest(canonicalize(dest));

        if (canonicalSource == canonicalDest)
        {
            dest = getNameOfCopy(source);
            canonicalDest = canonicalize(dest);
        }

        progress.setLabelText(tr("Copying %1 of %2\n\n"
                                 "%3\n\n"
                                 "to\n\n"
                                 "%4")
                              .arg(urlList.indexOf(url) + 1)
                              .arg(urlList.size())
                              .arg(canonicalSource)
                              .arg(canonicalDest));

        QMessageBox::StandardButton answer = checkOverwrite(dest);

        if (answer == QMessageBox::Cancel)
            break;

        if (answer == QMessageBox::No)
            continue;

        fileOp.setSource(source);
        fileOp.setDest(dest);

        if (fileOp.open())
        {
            qint64 copied = 0;
            qint64 totalCopied = 0;
            qint64 totalSize = fileOp.size();

            while (totalCopied < totalSize && !progress.wasCanceled())
            {
                copied = fileOp.copy();

                if (copied == -1)
                    break;

                if (copied == 0)
                    MSleep::msleep(10);

                totalCopied += copied;

                progress.setValue(totalCopied * 100 / totalSize);

                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            }

            fileOp.close();

            if (copied == -1)
                critical(tr("Failed to copy\n\n"
                            "%1\n\n"
                            "to\n\n"
                            "%2")
                         .arg(canonicalSource)
                         .arg(canonicalDest));
        }
    }
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

QMessageBox::StandardButton KFileWizard::checkOverwrite(const QString& dest)
{
    if (QFile(dest).exists())
    {
        return question(tr("%1\n\n"
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
    progress.setWindowTitle(tr("K File Wizard"));
    progress.setLabelText(tr("Deleting files..."));
    progress.setRange(0, urlList.size());
    progress.setModal(true);
    progress.setAutoClose(false);
    progress.show();

    FileOperation fileOp;

    foreach(QUrl url, urlList)
    {
        if (progress.wasCanceled())
            break;

        QString source(FileOperation::fixUrl(url.toString()));

        QString canonicalSource(canonicalize(source));

        progress.setLabelText(tr("Deleting %1 of %2\n\n"
                                 "%3\n\n")
                              .arg(urlList.indexOf(url) + 1)
                              .arg(urlList.size())
                              .arg(canonicalSource));

        fileOp.setSource(source);
        fileOp.open();

        if (!fileOp.remove())
            critical(tr("Failed to delete\n\n"
                        "%1")
                        .arg(canonicalSource));

        progress.setValue(urlList.indexOf(url));
    }
}

void KFileWizard::setLocationText(const QString& text, bool force)
{
    QString canonicalPath = canonicalize(text);

    if (force || canonicalPath != ui->locationLine->text())
    {
        ui->locationLine->setText(canonicalPath);

        locationReturnPressed(false);
    }
}

QString KFileWizard::canonicalize(const QString& path)
{
    return path.indexOf(':') > 1 ?
                FileOperation::fixUrl(QDir::fromNativeSeparators(path)) :
                QDir::toNativeSeparators(path);
}

void KFileWizard::locationReturnPressed(bool moveFocusToEntryView)
{
    QModelIndex current =
            dirProxyModel->mapFromSource(
                dirModel->index(ui->locationLine->text()));

    if (current.isValid())
    {
        // To canonicalize a path
        setLocationText(ui->locationLine->text());

        currentDir.setPath(ui->locationLine->text());

        ui->dirTree->setCurrentIndex(current);
        ui->dirTree->expand(current);
        ui->dirTree->scrollTo(current);

        setEntryRoot();

        if (moveFocusToEntryView)
            ui->entryTree->setFocus();
    }
    else
    {
        critical(tr("Directory is not valid."));

        ui->locationLine->selectAll();
    }
}

void KFileWizard::setEntryRoot()
{
    QModelIndex rootIndex = entryModel->index(currentDir.path());
    entryModel->setRootIndex(rootIndex);

    rootIndex = entryProxyModel->mapFromSource(rootIndex);
    ui->entryTree->setRootIndex(rootIndex);
}
