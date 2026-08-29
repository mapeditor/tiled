/*
 * filedependenciesdock.h
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

#pragma once

#include "filedependencies.h"

#include <QDockWidget>
#include <QList>

class QSortFilterProxyModel;
class QStandardItemModel;
class QTreeView;

namespace Tiled {

class Document;

class FileDependenciesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit FileDependenciesDock(QWidget *parent = nullptr);
    ~FileDependenciesDock() override;

    void setDocument(Document *document);

private:
    void doubleClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &pos);
    void openFile(const QString &filePath);
    void showInFileManager(const QString &filePath);

    QStandardItemModel *mModel;
    QSortFilterProxyModel *mProxyModel;
    QTreeView *mView;

    QList<FileReference> mReferences;
};

} // namespace Tiled
