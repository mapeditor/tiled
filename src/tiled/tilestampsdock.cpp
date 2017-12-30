/*
 * tilestampdock.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilestampsdock.h"

#include "documentmanager.h"
#include "preferences.h"
#include "tilestamp.h"
#include "tilestampmanager.h"
#include "tilestampmodel.h"
#include "utils.h"

#include <QAction>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QKeyEvent>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QVBoxLayout>

namespace Tiled {
namespace Internal {

TileStampsDock::TileStampsDock(TileStampManager *stampManager, QWidget *parent)
    : QDockWidget(parent)
    , mTileStampManager(stampManager)
    , mTileStampModel(stampManager->tileStampModel())
    , mProxyModel(new QSortFilterProxyModel(mTileStampModel))
    , mFilterEdit(new QLineEdit(this))
    , mNewStamp(new QAction(this))
    , mAddVariation(new QAction(this))
    , mDuplicate(new QAction(this))
    , mDelete(new QAction(this))
    , mChooseFolder(new QAction(this))
{
    setObjectName(QLatin1String("TileStampsDock"));

    mProxyModel->setSortLocaleAware(true);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSourceModel(mTileStampModel);
    mProxyModel->sort(0);

    mTileStampView = new TileStampView(this);
    mTileStampView->setModel(mProxyModel);
    mTileStampView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mTileStampView->header()->setStretchLastSection(false);
    mTileStampView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mTileStampView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    mTileStampView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mTileStampView, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(showContextMenu(QPoint)));

    mNewStamp->setIcon(QIcon(QLatin1String(":images/16x16/document-new.png")));
    mAddVariation->setIcon(QIcon(QLatin1String(":/images/16x16/add.png")));
    mDuplicate->setIcon(QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));
    mDelete->setIcon(QIcon(QLatin1String(":images/16x16/edit-delete.png")));
    mChooseFolder->setIcon(QIcon(QLatin1String(":images/16x16/document-open.png")));

    Utils::setThemeIcon(mNewStamp, "document-new");
    Utils::setThemeIcon(mAddVariation, "add");
    Utils::setThemeIcon(mDelete, "edit-delete");
    Utils::setThemeIcon(mChooseFolder, "document-open");

    mFilterEdit->setClearButtonEnabled(true);

    connect(mFilterEdit, &QLineEdit::textChanged,
            mProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    connect(mTileStampModel, &TileStampModel::stampRenamed,
            this, &TileStampsDock::ensureStampVisible);

    connect(mNewStamp, &QAction::triggered, this, &TileStampsDock::newStamp);
    connect(mAddVariation, &QAction::triggered, this, &TileStampsDock::addVariation);
    connect(mDuplicate, &QAction::triggered, this, &TileStampsDock::duplicate);
    connect(mDelete, &QAction::triggered, this, &TileStampsDock::delete_);
    connect(mChooseFolder, &QAction::triggered, this, &TileStampsDock::chooseFolder);

    mDuplicate->setEnabled(false);
    mDelete->setEnabled(false);
    mAddVariation->setEnabled(false);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    QToolBar *buttonContainer = new QToolBar;
    buttonContainer->setFloatable(false);
    buttonContainer->setMovable(false);
    buttonContainer->setIconSize(Utils::smallIconSize());

    buttonContainer->addAction(mNewStamp);
    buttonContainer->addAction(mAddVariation);
    buttonContainer->addAction(mDuplicate);
    buttonContainer->addAction(mDelete);

    QWidget *stretch = new QWidget;
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonContainer->addWidget(stretch);

    buttonContainer->addAction(mChooseFolder);

    QVBoxLayout *listAndToolBar = new QVBoxLayout;
    listAndToolBar->setSpacing(0);
    listAndToolBar->addWidget(mFilterEdit);
    listAndToolBar->addWidget(mTileStampView);
    listAndToolBar->addWidget(buttonContainer);

    layout->addLayout(listAndToolBar);

    QItemSelectionModel *selectionModel = mTileStampView->selectionModel();
    connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentRowChanged(QModelIndex)));

    setWidget(widget);
    retranslateUi();
}

void TileStampsDock::changeEvent(QEvent *e)
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

void TileStampsDock::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        delete_();
        return;
    }

    QDockWidget::keyPressEvent(event);
}

void TileStampsDock::currentRowChanged(const QModelIndex &index)
{
    const QModelIndex sourceIndex = mProxyModel->mapToSource(index);
    const bool isStamp = mTileStampModel->isStamp(sourceIndex);

    mDuplicate->setEnabled(isStamp);
    mDelete->setEnabled(sourceIndex.isValid());
    mAddVariation->setEnabled(isStamp);

    if (isStamp) {
        emit setStamp(mTileStampModel->stampAt(sourceIndex));
    } else if (const TileStampVariation *variation = mTileStampModel->variationAt(sourceIndex)) {
        // single variation clicked, use it specifically
        emit setStamp(TileStamp(new Map(*variation->map)));
    }
}

void TileStampsDock::showContextMenu(QPoint pos)
{
    const QModelIndex index = mTileStampView->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu;

    const QModelIndex sourceIndex = mProxyModel->mapToSource(index);
    if (mTileStampModel->isStamp(sourceIndex)) {
        QAction *addStampVariation = new QAction(mAddVariation->icon(),
                                                 mAddVariation->text(), &menu);
        QAction *deleteStamp = new QAction(mDelete->icon(),
                                           tr("Delete Stamp"), &menu);

        connect(deleteStamp, SIGNAL(triggered(bool)), SLOT(delete_()));
        connect(addStampVariation, SIGNAL(triggered(bool)), SLOT(addVariation()));

        menu.addAction(addStampVariation);
        menu.addSeparator();
        menu.addAction(deleteStamp);
    } else {
        QAction *removeVariation = new QAction(QIcon(QLatin1String(":/images/16x16/remove.png")),
                                               tr("Remove Variation"),
                                               &menu);

        Utils::setThemeIcon(removeVariation, "remove");

        connect(removeVariation, SIGNAL(triggered(bool)), SLOT(delete_()));

        menu.addAction(removeVariation);
    }

    menu.exec(mTileStampView->viewport()->mapToGlobal(pos));
}

void TileStampsDock::newStamp()
{
    TileStamp stamp = mTileStampManager->createStamp();

    if (isVisible() && !stamp.isEmpty()) {
        QModelIndex stampIndex = mTileStampModel->index(stamp);
        if (stampIndex.isValid()) {
            QModelIndex viewIndex = mProxyModel->mapFromSource(stampIndex);
            mTileStampView->setCurrentIndex(viewIndex);
            mTileStampView->edit(viewIndex);
        }
    }
}

void TileStampsDock::delete_()
{
    const QModelIndex index = mTileStampView->currentIndex();
    if (!index.isValid())
        return;

    const QModelIndex sourceIndex = mProxyModel->mapToSource(index);
    mTileStampModel->removeRow(sourceIndex.row(), sourceIndex.parent());
}

void TileStampsDock::duplicate()
{
    const QModelIndex index = mTileStampView->currentIndex();
    if (!index.isValid())
        return;

    const QModelIndex sourceIndex = mProxyModel->mapToSource(index);
    if (!mTileStampModel->isStamp(sourceIndex))
        return;

    TileStamp stamp = mTileStampModel->stampAt(sourceIndex);
    mTileStampModel->addStamp(stamp.clone());
}

void TileStampsDock::addVariation()
{
    const QModelIndex index = mTileStampView->currentIndex();
    if (!index.isValid())
        return;

    const QModelIndex sourceIndex = mProxyModel->mapToSource(index);
    if (!mTileStampModel->isStamp(sourceIndex))
        return;

    const TileStamp &stamp = mTileStampModel->stampAt(sourceIndex);
    mTileStampManager->addVariation(stamp);
}

void TileStampsDock::chooseFolder()
{
    Preferences *prefs = Preferences::instance();

    QString stampsDirectory = prefs->stampsDirectory();
    stampsDirectory = QFileDialog::getExistingDirectory(window(),
                                                        tr("Choose the Stamps Folder"),
                                                        stampsDirectory);
    if (!stampsDirectory.isEmpty())
        prefs->setStampsDirectory(stampsDirectory);
}

void TileStampsDock::ensureStampVisible(const TileStamp &stamp)
{
    QModelIndex stampIndex = mTileStampModel->index(stamp);
    if (stampIndex.isValid())
        mTileStampView->scrollTo(mProxyModel->mapFromSource(stampIndex));
}

void TileStampsDock::retranslateUi()
{
    setWindowTitle(tr("Tile Stamps"));

    mNewStamp->setText(tr("Add New Stamp"));
    mAddVariation->setText(tr("Add Variation"));
    mDuplicate->setText(tr("Duplicate Stamp"));
    mDelete->setText(tr("Delete Selected"));
    mChooseFolder->setText(tr("Set Stamps Folder"));

    mFilterEdit->setPlaceholderText(tr("Filter"));
}


TileStampView::TileStampView(QWidget *parent)
    : QTreeView(parent)
{
}

QSize TileStampView::sizeHint() const
{
    return Utils::dpiScaled(QSize(200, 200));
}

bool TileStampView::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Tab) {
            if (indexWidget(currentIndex())) {
                event->accept();
                return true;
            }
        }
    }

    return QTreeView::event(event);
}

} // namespace Internal
} // namespace Tiled
