/*
 * filedependenciesdock.cpp
 * Copyright 2024, File Dependencies Contributor
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "filedependenciesdock.h"

#include "documentmanager.h"
#include "mapdocument.h"
#include "tilesetdocument.h"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace Tiled {

FileDependenciesDock::FileDependenciesDock(QWidget *parent)
    : QDockWidget(parent)
    , mModel(new QStandardItemModel(0, 3, this))
    , mProxyModel(new QSortFilterProxyModel(this))
    , mView(new QTreeView(this))
{
    setWindowTitle(tr("File Dependencies"));
    setObjectName(QLatin1String("fileDependenciesDock"));

    mModel->setHeaderData(0, Qt::Horizontal, tr("File name"));
    mModel->setHeaderData(1, Qt::Horizontal, tr("Location"));
    mModel->setHeaderData(2, Qt::Horizontal, tr("Type"));

    mProxyModel->setSourceModel(mModel);
    mProxyModel->setSortLocaleAware(true);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    mView->setModel(mProxyModel);
    mView->setRootIsDecorated(false);
    mView->setItemsExpandable(false);
    mView->setSortingEnabled(true);
    mView->setSelectionMode(QAbstractItemView::SingleSelection);
    mView->sortByColumn(0, Qt::AscendingOrder);

    mView->header()->setStretchLastSection(false);
    mView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    mView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    mView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    mView->setContextMenuPolicy(Qt::CustomContextMenu);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mView);
    setWidget(widget);

    connect(mView, &QTreeView::doubleClicked, this, &FileDependenciesDock::doubleClicked);
    connect(mView, &QTreeView::customContextMenuRequested, this, &FileDependenciesDock::showContextMenu);
}

FileDependenciesDock::~FileDependenciesDock()
{
}

void FileDependenciesDock::setDocument(Document *document)
{
    mModel->removeRows(0, mModel->rowCount());
    mReferences.clear();

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        mReferences = FileDependencyCollector::collectFromMap(mapDocument->map());
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        mReferences = FileDependencyCollector::collectFromTileset(tilesetDocument->tileset().data());
    }

    mModel->setRowCount(mReferences.size());
    for (int i = 0; i < mReferences.size(); ++i) {
        const FileReference &ref = mReferences.at(i);
        QFileInfo info(ref.filePath);

        auto nameItem = new QStandardItem(info.fileName());
        nameItem->setEditable(false);
        auto pathItem = new QStandardItem(QDir::toNativeSeparators(info.path()));
        pathItem->setEditable(false);
        auto typeItem = new QStandardItem(ref.type);
        typeItem->setEditable(false);

        mModel->setItem(i, 0, nameItem);
        mModel->setItem(i, 1, pathItem);
        mModel->setItem(i, 2, typeItem);
    }
}

void FileDependenciesDock::doubleClicked(const QModelIndex &proxyIndex)
{
    const auto index = mProxyModel->mapToSource(proxyIndex);
    const FileReference &ref = mReferences.at(index.row());

    if (ref.canOpenInTiled) {
        openFile(ref.filePath);
    }
}

void FileDependenciesDock::showContextMenu(const QPoint &pos)
{
    QModelIndex proxyIndex = mView->indexAt(pos);
    if (!proxyIndex.isValid())
        return;

    const auto index = mProxyModel->mapToSource(proxyIndex);
    const FileReference &ref = mReferences.at(index.row());

    QMenu menu(this);

    if (ref.canOpenInTiled) {
        QAction *openAction = menu.addAction(tr("Open in Tiled"));
        connect(openAction, &QAction::triggered, this, [this, filePath = ref.filePath]() {
            openFile(filePath);
        });
    }

    QAction *showManagerAction = menu.addAction(tr("Show in File Manager"));
    connect(showManagerAction, &QAction::triggered, this, [this, filePath = ref.filePath]() {
        showInFileManager(filePath);
    });

    menu.exec(mView->viewport()->mapToGlobal(pos));
}

void FileDependenciesDock::openFile(const QString &filePath)
{
    DocumentManager::instance()->openFile(filePath);
}

void FileDependenciesDock::showInFileManager(const QString &filePath)
{
    QFileInfo info(filePath);
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
}

} // namespace Tiled
