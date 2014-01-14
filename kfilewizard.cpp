#include <QtGui>

#include "kfilewizard.h"
#include "ui_kfilewizard.h"

#include "filesystemsortfilterproxymodel.h"
#include "entrylistmodel.h"
#include "entrytreeview.h"

KFileWizard::KFileWizard(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KFileWizard),
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

void KFileWizard::initLocationLine()
{
    connect(ui->locationLine, SIGNAL(returnPressed()),
            this, SLOT(locationReturnPressed()));
}

void KFileWizard::initSplitter()
{
    // provide more space to a right widget, EntryTreeView
    ui->splitter->setStretchFactor(1, 1);
}

void KFileWizard::initDirTree()
{
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

    connect(ui->entryTree, SIGNAL(activated(QModelIndex)),
            this, SLOT(entryActivated(QModelIndex)));

    connect(ui->entryTree, SIGNAL(cdUp(QModelIndex)), this, SLOT(entryCdUp(QModelIndex)));

    setEntryRoot();
}

void KFileWizard::dirLoaded(const QString& dir)
{
    Q_UNUSED(dir);

    QModelIndex current = dirProxyModel->mapFromSource(dirModel->index(currentDir.path()));

    dirActivated(current);
}

void KFileWizard::dirActivated(const QModelIndex &index)
{
    setLocationText(dirModel->filePath(dirProxyModel->mapToSource(index)), true);
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
                            entryModel->parent(entryProxyModel->mapToSource(index))));
}

void KFileWizard::setLocationText(const QString& text, bool force)
{
    QString canonicalPath = canonicalize(text);

    if (force || canonicalPath != ui->locationLine->text())
    {
        ui->locationLine->setText(canonicalPath);

        locationReturnPressed();
    }
}

QString KFileWizard::canonicalize(const QString& path)
{
    QString result(path);

    if (result.startsWith("ftp:"))
    {
        result = QDir::fromNativeSeparators(result);

        // QFileSystemModel does not recognize URL correctly.
        // Always use xxx:/yyy style for a URL as well as a local path
        int index = result.indexOf(":/");
        if (index > 1 && result.at(index + QString(":/").length()) != '/')
            result.replace(index, QString(":/").length(), "://");
    }
    else
        result = QDir::toNativeSeparators(result);

    return result;
}

void KFileWizard::locationReturnPressed()
{
    QModelIndex current = dirProxyModel->mapFromSource(dirModel->index(ui->locationLine->text()));

    if (current.isValid())
    {
        // To canonicalize a path
        setLocationText(ui->locationLine->text());

        currentDir.setPath(ui->locationLine->text());

        ui->dirTree->setCurrentIndex(current);
        ui->dirTree->expand(current);
        ui->dirTree->scrollTo(current);

        setEntryRoot();
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
