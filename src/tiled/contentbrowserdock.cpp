#include "contentbrowserdock.h"

#include "documentmanager.h"
#include "mainwindow.h"

#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSplitter>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QMimeData>
#include <QDrag>
#include <QPixmap>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>

namespace Tiled {

ContentBrowserDock::ContentBrowserDock(QWidget *parent)
    : QDockWidget(tr("Content Browser"), parent)
{
    setFeatures(QDockWidget::DockWidgetClosable |
                QDockWidget::DockWidgetMovable |
                QDockWidget::DockWidgetFloatable);

    //
    // File Model
    //
    mFileModel = new QFileSystemModel(this);
    mFileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    mFileModel->setNameFilterDisables(false);

    QStringList nameFilters = {
        QStringLiteral("*.png"),
        QStringLiteral("*.jpg"),
        QStringLiteral("*.jpeg"),
        QStringLiteral("*.bmp"),
        QStringLiteral("*.tsx"),
        QStringLiteral("*.json"),
        QStringLiteral("*.tmx")
    };
    mFileModel->setNameFilters(nameFilters);

    //
    // Proxy Models (Tree + Details)
    //
    mTreeProxy = new QSortFilterProxyModel(this);
    mTreeProxy->setSourceModel(mFileModel);
    mTreeProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    mDetailsProxy = new QSortFilterProxyModel(this);
    mDetailsProxy->setSourceModel(mFileModel);
    mDetailsProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    //
    // Filter Box
    //
    mFilterEdit = new QLineEdit(this);
    mFilterEdit->setPlaceholderText(tr("Filter assets..."));

    //
    // Tree View
    //
    mTreeView = new QTreeView(this);
    mTreeView->setHeaderHidden(true);
    mTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mTreeView->setDragEnabled(true);
    mTreeView->setModel(mTreeProxy);

    //
    // Details (List) View
    //
    mDetailsView = new QListView(this);
    mDetailsView->setViewMode(QListView::IconMode);
    mDetailsView->setResizeMode(QListView::Adjust);
    mDetailsView->setIconSize(QSize(96, 96));
    mDetailsView->setGridSize(QSize(128, 128));
    mDetailsView->setSpacing(12);
    mDetailsView->setDragEnabled(true);
    mDetailsView->setModel(mDetailsProxy);

    //
    // Split View
    //
    mSplitter = new QSplitter(Qt::Horizontal, this);
    mSplitter->addWidget(mTreeView);
    mSplitter->addWidget(mDetailsView);
    mSplitter->setSizes({200, 600});

    //
    // Layout
    //
    auto *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);
    mainLayout->addWidget(mFilterEdit);
    mainLayout->addWidget(mSplitter);

    QWidget *container = new QWidget(this);
    container->setLayout(mainLayout);
    setWidget(container);

    //
    // Connections
    //
    connect(mFilterEdit, &QLineEdit::textChanged,
            this, &ContentBrowserDock::onFilterChanged);

    connect(mTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &) {
                if (current.isValid())
                    onTreeClicked(current);
            });

    connect(mDetailsView, &QListView::doubleClicked,
            this, &ContentBrowserDock::onDetailsDoubleClicked);

    //
    // Determine root assets path
    //
    QString rootPath = QCoreApplication::applicationDirPath() + QStringLiteral("/assets");

    QSettings settings(QStringLiteral("IonixEngine2025"), QStringLiteral("ContentBrowser"));
    QString customRoot = settings.value("projectAssetsPath").toString();

    if (!customRoot.isEmpty() && QDir(customRoot).exists())
    {
        rootPath = customRoot;
        qDebug() << "Using custom project path:" << rootPath;
    }

    if (!QDir(rootPath).exists())
    {
        rootPath = QDir::currentPath();
        qDebug() << "Assets folder not found, using current dir:" << rootPath;
    }

    //
    // Apply root to views
    //
    mFileModel->setRootPath(rootPath);
    mTreeView->setRootIndex(mTreeProxy->mapFromSource(mFileModel->index(rootPath)));
    navigateToFolder(rootPath);
}

//
// Navigation
//
void ContentBrowserDock::navigateToFolder(const QString &path)
{
    mCurrentFolder = path;

    QModelIndex sourceIndex = mFileModel->index(path);
    QModelIndex treeIndex = mTreeProxy->mapFromSource(sourceIndex);
    QModelIndex detailsIndex = mDetailsProxy->mapFromSource(sourceIndex);

    mTreeView->setCurrentIndex(treeIndex);
    mDetailsView->setRootIndex(detailsIndex);
}

//
// Tree click (folder navigation)
//
void ContentBrowserDock::onTreeClicked(const QModelIndex &proxyIndex)
{
    QModelIndex sourceIndex = mTreeProxy->mapToSource(proxyIndex);
    QString path = mFileModel->filePath(sourceIndex);

    if (QFileInfo(path).isDir())
        navigateToFolder(path);
}

//
// Details double-click (open file OR enter folder)
//
void ContentBrowserDock::onDetailsDoubleClicked(const QModelIndex &proxyIndex)
{
    QModelIndex sourceIndex = mDetailsProxy->mapToSource(proxyIndex);
    QString path = mFileModel->filePath(sourceIndex);
    QFileInfo info(path);

    if (info.isDir())
    {
        navigateToFolder(path);
        return;
    }

    DocumentManager::instance()->openFile(path);

    if (auto *mainWindow = MainWindow::instance())
        mainWindow->raise();
}

//
// Filtering
//
void ContentBrowserDock::onFilterChanged(const QString &text)
{
    QString pattern = text.isEmpty()
                      ? QString()
                      : QStringLiteral("*") + text + QStringLiteral("*");

    mTreeProxy->setFilterWildcard(pattern);
    mDetailsProxy->setFilterWildcard(pattern);
}

} // namespace Tiled
