/*
 * mapsdock.cpp
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
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

#include "mapsdock.h"

#include "documentmanager.h"
#include "mapformat.h"
#include "pluginmanager.h"
#include "preferences.h"
#include "utils.h"

#include <QBoxLayout>
#include <QCompleter>
#include <QDirModel>
#include <QEvent>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>

using namespace Tiled;
using namespace Tiled::Internal;

/**
 * Class represents the file system model with disabled dragging of directories.
 */
class FileSystemModel : public QFileSystemModel
{
public:
    explicit FileSystemModel(QObject *parent = nullptr):
        QFileSystemModel(parent)
    {
    }

    Qt::ItemFlags flags(const QModelIndex &i) const override
    {
        Qt::ItemFlags flags = QFileSystemModel::flags(i);
        if (isDir(i))
            flags &= ~Qt::ItemIsDragEnabled;
        return flags;
    }
};

MapsDock::MapsDock(QWidget *parent)
    : QDockWidget(parent)
    , mDirectoryEdit(new QLineEdit)
    , mMapsView(new MapsView)
{
    setObjectName(QLatin1String("MapsDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    QHBoxLayout *dirLayout = new QHBoxLayout;

    // QDirModel is obsolete, but I could not get QFileSystemModel to work here
    QDirModel *model = new QDirModel(this);
    model->setFilter(QDir::AllDirs | QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot);
    QCompleter *completer = new QCompleter(model, this);
    mDirectoryEdit->setCompleter(completer);

    QPushButton *button = new QPushButton(tr("Browse..."));
    dirLayout->addWidget(mDirectoryEdit);
    dirLayout->addWidget(button);

    layout->addWidget(mMapsView);
    layout->addLayout(dirLayout);

    setWidget(widget);
    retranslateUi();

    connect(button, &QAbstractButton::clicked, this, &MapsDock::browse);

    Preferences *prefs = Preferences::instance();
    connect(prefs, &Preferences::mapsDirectoryChanged, this, &MapsDock::onMapsDirectoryChanged);

    mDirectoryEdit->setText(prefs->mapsDirectory());
    connect(mDirectoryEdit, &QLineEdit::returnPressed, this, &MapsDock::editedMapsDirectory);
}

void MapsDock::browse()
{
    QString f = QFileDialog::getExistingDirectory(window(),
                                                  tr("Choose the Maps Folder"),
                                                  mDirectoryEdit->text());
    if (!f.isEmpty()) {
        Preferences *prefs = Preferences::instance();
        prefs->setMapsDirectory(f);
    }
}

void MapsDock::editedMapsDirectory()
{
    Preferences *prefs = Preferences::instance();

    const QFileInfo fileInfo(mDirectoryEdit->text());
    if (fileInfo.isDir()) {
        prefs->setMapsDirectory(fileInfo.filePath());
    } else if (fileInfo.isFile()) {
        mDirectoryEdit->setText(fileInfo.path());
        prefs->setMapsDirectory(fileInfo.path());
    }
}

void MapsDock::onMapsDirectoryChanged()
{
    Preferences *prefs = Preferences::instance();
    mDirectoryEdit->setText(prefs->mapsDirectory());
}

void MapsDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void MapsDock::retranslateUi()
{
    setWindowTitle(tr("Maps"));
}

///// ///// ///// ///// /////

MapsView::MapsView(QWidget *parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setDragEnabled(true);
    setDefaultDropAction(Qt::MoveAction);

    Preferences *prefs = Preferences::instance();
    connect(prefs, &Preferences::mapsDirectoryChanged,
            this, &MapsView::onMapsDirectoryChanged);

    QDir mapsDir(prefs->mapsDirectory());
    if (!mapsDir.exists())
        mapsDir.setPath(QDir::currentPath());

    mFileSystemModel = new FileSystemModel(this);
    mFileSystemModel->setRootPath(mapsDir.absolutePath());

    QStringList nameFilters;

    for (MapFormat *format : PluginManager::objects<MapFormat>()) {
        if (!(format->capabilities() & MapFormat::Read))
            continue;

        const QString filter = format->nameFilter();
        nameFilters.append(Utils::cleanFilterList(filter));
    }

    mFileSystemModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDot);
    mFileSystemModel->setNameFilters(nameFilters);
    mFileSystemModel->setNameFilterDisables(false); // hide filtered files

    setModel(mFileSystemModel);

    QHeaderView *headerView = header();
    headerView->hideSection(1); // Size column
    headerView->hideSection(2); // Type column
    headerView->hideSection(3); // Modified column

    setRootIndex(mFileSystemModel->index(mapsDir.absolutePath()));

    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(this, &QAbstractItemView::activated,
            this, &MapsView::onActivated);
}

QSize MapsView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void MapsView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        // Prevent drag-and-drop starting when clicking on an unselected item.
        setDragEnabled(selectionModel()->isSelected(index));
    }

    QTreeView::mousePressEvent(event);
}

void MapsView::onMapsDirectoryChanged()
{
    Preferences *prefs = Preferences::instance();
    QDir mapsDir(prefs->mapsDirectory());
    if (!mapsDir.exists())
        mapsDir.setPath(QDir::currentPath());
    model()->setRootPath(mapsDir.absolutePath());
    setRootIndex(model()->index(mapsDir.absolutePath()));
}

void MapsView::onActivated(const QModelIndex &index)
{
    QString path = model()->filePath(index);
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        Preferences *prefs = Preferences::instance();
        prefs->setMapsDirectory(fileInfo.absoluteFilePath());
        return;
    }

    DocumentManager::instance()->openFile(path);
}
