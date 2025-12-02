#include "contentbrowserdock.h"

#include <QFileSystemModel>
#include <QTreeView>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QMimeData>
#include <QDrag>
#include <QMouseEvent>
#include <QPixmap>

namespace Tiled {

ContentBrowserDock::ContentBrowserDock(QWidget *parent)
    : QDockWidget(tr("Content Browser Dock"), parent)
{

    mFileModel = new QFileSystemModel(this);
    mFileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

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
    mFileModel->setNameFilterDisables(false);


    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSourceModel(mFileModel);
    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setFilterKeyColumn(0);


    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);    mTreeView = new QTreeView(this);
    mTreeView->setModel(mProxyModel);
    mTreeView->setHeaderHidden(true);
    mTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mTreeView->setDragEnabled(true);
    mTreeView->setDragDropMode(QAbstractItemView::DragOnly);


    mFilterEdit = new QLineEdit(this);
    mFilterEdit->setPlaceholderText(tr("Filter assets..."));
    connect(mFilterEdit, &QLineEdit::textChanged,
            this, &ContentBrowserDock::onFilterChanged);


    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mFilterEdit);
    layout->addWidget(mTreeView);

    QWidget *container = new QWidget(this);
    container->setLayout(layout);
    setWidget(container);


    QString root = QDir::currentPath() + QStringLiteral("/assets");
    if (!QDir(root).exists())
        root = QDir::currentPath();

    mFileModel->setRootPath(root);
    mTreeView->setRootIndex(mProxyModel->mapFromSource(
                                mFileModel->index(root)));
}

void ContentBrowserDock::onFilterChanged(const QString &text)
{
    mProxyModel->setFilterWildcard(text);
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

