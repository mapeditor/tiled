#include "contentbrowserdock.h"

#include "documentmanager.h"
#include "mainwindow.h"
#include "projectdock.h"
#include "project.h"

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
#include <QMouseEvent>
#include <QPixmap>

namespace Tiled {

ContentBrowserDock::ContentBrowserDock(QWidget *parent)
    : QDockWidget(tr("Content Browser"), parent)
{

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


    mTreeProxy = new QSortFilterProxyModel(this);
    mTreeProxy->setSourceModel(mFileModel);
    mTreeProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

<<<<<<< Updated upstream
<<<<<<< Updated upstream

    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);    mTreeView = new QTreeView(this);
    mTreeView->setModel(mProxyModel);
    mTreeView->setHeaderHidden(true);
    mTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mTreeView->setDragEnabled(true);
    mTreeView->setDragDropMode(QAbstractItemView::DragOnly);
=======
    mDetailsProxy = new QSortFilterProxyModel(this);
    mDetailsProxy->setSourceModel(mFileModel);
    mDetailsProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
>>>>>>> Stashed changes
=======
    mDetailsProxy = new QSortFilterProxyModel(this);
    mDetailsProxy->setSourceModel(mFileModel);
    mDetailsProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
>>>>>>> Stashed changes


    mFilterEdit = new QLineEdit(this);
    mFilterEdit->setPlaceholderText(tr("Filter assets..."));
    //we use to have just 1 proxy model now we have 2 (tree and list) therefore we must rename the creation of the ui widgets and make 2 1 for the tree view (which we had) and a new one for the list view.
    mTreeView = new QTreeView(this);
    mTreeView->setHeaderHidden(true);
    mTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mTreeView->setDragEnabled(true);
    mTreeView->setModel(mTreeProxy);

    mDetailsView = new QListView(this);
    mDetailsView->setViewMode(QListView::IconMode);
    mDetailsView->setResizeMode(QListView::Adjust);
    mDetailsView->setIconSize(QSize(96, 96));
    mDetailsView->setGridSize(QSize(128,128));
    mDetailsView->setSpacing(12);
    mDetailsView->setDragEnabled(true);
    mDetailsView->setModel(mDetailsProxy);

    mSplitter = new QSplitter(Qt::Horizontal, this);
    mSplitter->addWidget(mTreeView);
    mSplitter->addWidget(mDetailsView);
    mSplitter->setSizes({200,600});


    auto *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);
    mainLayout->addWidget(mFilterEdit);
    mainLayout->addWidget(mSplitter);

    QWidget *container = new QWidget(this);
    container->setLayout(mainLayout);
    setWidget(container);

    connect(mFilterEdit, &QLineEdit::textChanged, this, &ContentBrowserDock::onFilterChanged);
    connect(mTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &current, const QModelIndex &)
    {
        if(current.isValid())
            onTreeClicked(current);
    });
    connect(mDetailsView, &QListView::doubleClicked, this, &ContentBrowserDock::onDetailsDoubleClicked);

    QString rootPath = QCoreApplication::applicationDirPath() + QStringLiteral("/assets");  // Relative to Tiled.exe

    // Optional: Override with a config file (e.g., for your game project)
    QSettings settings(QStringLiteral("YourGameEngine"), QStringLiteral("ContentBrowser"));  // Change "YourGameEngine" to your app name
    QString customRoot = settings.value("projectAssetsPath").toString();
    if (!customRoot.isEmpty() && QDir(customRoot).exists()) {
        rootPath = customRoot;
        qDebug() << "Using custom project path:" << rootPath;
    }

    if (!QDir(rootPath).exists()) {
        rootPath = QDir::currentPath();  // Fallback to current dir
        qDebug() << "Assets folder not found, using current dir:" << rootPath;
    }

    mFileModel->setRootPath(rootPath);
    mTreeView->setRootIndex(mTreeProxy->mapFromSource(mFileModel->index(rootPath)));
    navigateToFolder(rootPath);
}

void ContentBrowserDock::navigateToFolder(const QString &path)
{
    mCurrentFolder = path;

    QModelIndex sourceIndex = mFileModel->index(path);
    QModelIndex treeIndex = mTreeProxy->mapFromSource(sourceIndex);
    QModelIndex detailsIndex = mDetailsProxy->mapFromSource(sourceIndex);

    mTreeView->setCurrentIndex(treeIndex);
    mDetailsView->setCurrentIndex(detailsIndex);
}

void ContentBrowserDock::onTreeClicked(const QModelIndex &proxyIndex)
{
    QModelIndex sourceIndex = mTreeProxy->mapToSource(proxyIndex);
    QString path = mFileModel->filePath(sourceIndex);

    if(QFileInfo(path).isDir())
    {
        navigateToFolder(path);
    }
}

void ContentBrowserDock::onDetailsDoubleClicked(const QModelIndex &proxyIndex)
{
    QModelIndex sourceIndex = mDetailsProxy->mapToSource(proxyIndex);
    QString path = mFileModel->filePath(sourceIndex);
    QFileInfo info(path);

    if(info.isDir())
    {
        navigateToFolder(path);
        return;
    }

    #include "documentmanager.h"
    #include "mainwindow.h"
    using namespace Tiled;
    DocumentManager::instance()->openFile(path);

    if(auto *mainWindow = Tiled::MainWindow::instance())
    {
        mainWindow->raise();
    }
}
void ContentBrowserDock::onFilterChanged(const QString &text)
{
    QString pattern = text.isEmpty() ? QString() : QStringLiteral("*") + text + QStringLiteral("*");
    mTreeProxy->setFilterWildcard(pattern);
    mDetailsProxy->setFilterWildcard(pattern);
}


class TreeViewWithDrag : public QTreeView
{
protected:
    void startDrag(Qt::DropActions supportedActions) override
    {
        Q_UNUSED(supportedActions);
        QModelIndex index = currentIndex();
        if (!index.isValid())
            return;

        QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel*>(model());
        QFileSystemModel *fsModel = qobject_cast<QFileSystemModel*>(proxy->sourceModel());
        QString path = fsModel->filePath(proxy->mapToSource(index));

        QMimeData *mime = new QMimeData;
        mime->setUrls({QUrl::fromLocalFile(path)});

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mime);

        // optional thumbnail - something to add later
        if (path.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive) ||
            path.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive)) {
            QPixmap pix(path);
            if (!pix.isNull())
                drag->setPixmap(pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        drag->exec(Qt::CopyAction);
    }
};

}

