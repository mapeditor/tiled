/*
 * tileanimationeditor.cpp
 * Copyright 2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tileanimationeditor.h"
#include "ui_tileanimationeditor.h"

#include "changetileanimation.h"
#include "mapdocument.h"
#include "rangeset.h"
#include "tile.h"
#include "tiled.h"
#include "tileset.h"
#include "utils.h"

#include <QAbstractListModel>
#include <QCloseEvent>
#include <QShortcut>
#include <QUndoStack>
#include <QMimeData>

namespace Tiled {
namespace Internal {

class FrameListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FrameListModel(QObject *parent = 0)
        : QAbstractListModel(parent)
    {}

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool removeRows(int row, int count, const QModelIndex &parent);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column,
                      const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    void setFrames(const Tileset *tileset, const QVector<Frame> &frames);
    const QVector<Frame> &frames() const;

private:
    const Tileset *mTileset;
    QVector<Frame> mFrames;
};

int FrameListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mFrames.size();
}

QVariant FrameListModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return mFrames.at(index.row()).duration;
    case Qt::DecorationRole: {
        int tileId = mFrames.at(index.row()).tileId;
        if (Tile *tile = mTileset->tileAt(tileId))
            return tile->image();
    }
    }

    return QVariant();
}

bool FrameListModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
    if (role == Qt::EditRole) {
        int duration = value.toInt();
        if (duration >= 0) {
            mFrames[index.row()].duration = duration;
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

Qt::ItemFlags FrameListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsEditable | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

bool FrameListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!parent.isValid()) {
        if (count > 0) {
            beginRemoveRows(parent, row, row + count - 1);
            mFrames.remove(row, count);
            endRemoveRows();
        }
        return true;
    }

    return false;
}

Qt::DropActions FrameListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList FrameListModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String(TILES_MIMETYPE);
    types << QLatin1String(FRAMES_MIMETYPE);
    return types;
}

QMimeData *FrameListModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            const Frame &frame = mFrames.at(index.row());
            stream << frame.tileId;
            stream << frame.duration;
        }
    }

    mimeData->setData(QLatin1String(FRAMES_MIMETYPE), encodedData);
    return mimeData;
}

bool FrameListModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                  int row, int column,
                                  const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (column > 0)
        return false;

    int beginRow;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = mFrames.size();

    QVector<Frame> newFrames;

    if (data->hasFormat(QLatin1String(FRAMES_MIMETYPE))) {
        QByteArray encodedData = data->data(QLatin1String(FRAMES_MIMETYPE));
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            Frame frame;
            stream >> frame.tileId;
            stream >> frame.duration;
            newFrames.append(frame);
        }
    } else if (data->hasFormat(QLatin1String(TILES_MIMETYPE))) {
        QByteArray encodedData = data->data(QLatin1String(TILES_MIMETYPE));
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            Frame frame;
            stream >> frame.tileId;
            frame.duration = 100;
            newFrames.append(frame);
        }
    }

    if (newFrames.isEmpty())
        return false;

    beginInsertRows(QModelIndex(), beginRow, beginRow + newFrames.size() - 1);

    mFrames.insert(beginRow, newFrames.size(), Frame());
    for (int i = 0; i < newFrames.size(); ++i)
        mFrames[i + beginRow] = newFrames[i];

    endInsertRows();

    return true;
}

void FrameListModel::setFrames(const Tileset *tileset,
                               const QVector<Frame> &frames)
{
    beginResetModel();
    mTileset = tileset;
    mFrames = frames;
    endResetModel();
}

const QVector<Frame> &FrameListModel::frames() const
{
    return mFrames;
}


TileAnimationEditor::TileAnimationEditor(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , mUi(new Ui::TileAnimationEditor)
    , mMapDocument(0)
    , mTile(0)
    , mFrameListModel(new FrameListModel(this))
    , mApplyingChanges(false)
{
    mUi->setupUi(this);
    mUi->frameList->setModel(mFrameListModel);
    mUi->tilesetView->setMarkAnimatedTiles(false);

    connect(mFrameListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(framesEdited()));
    connect(mFrameListModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(framesEdited()));
    connect(mFrameListModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(framesEdited()));
    connect(mFrameListModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            SLOT(framesEdited()));

    QShortcut *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    QShortcut *redoShortcut = new QShortcut(QKeySequence::Redo, this);
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete, this);

    connect(undoShortcut, SIGNAL(activated()), SLOT(undo()));
    connect(redoShortcut, SIGNAL(activated()), SLOT(redo()));
    connect(deleteShortcut, SIGNAL(activated()), SLOT(delete_()));

    Utils::restoreGeometry(this);
}

TileAnimationEditor::~TileAnimationEditor()
{
    delete mUi;
}

void TileAnimationEditor::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;
    mUi->tilesetView->setMapDocument(mapDocument);

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(tileAnimationChanged(Tile*)),
                SLOT(tileAnimationChanged(Tile*)));
    }
}

void TileAnimationEditor::setTile(Tile *tile)
{
    mTile = tile;
    delete mUi->tilesetView->model();

    if (tile) {
        mFrameListModel->setFrames(tile->tileset(), tile->frames());

        TilesetModel *tilesetModel = new TilesetModel(tile->tileset(),
                                                      mUi->tilesetView);
        mUi->tilesetView->setModel(tilesetModel);
    } else {
        mFrameListModel->setFrames(0, QVector<Frame>());
    }
}

void TileAnimationEditor::writeSettings()
{
    Utils::saveGeometry(this);
}

void TileAnimationEditor::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}

void TileAnimationEditor::framesEdited()
{
    QUndoStack *undoStack = mMapDocument->undoStack();

    mApplyingChanges = true;
    undoStack->push(new ChangeTileAnimation(mMapDocument,
                                            mTile,
                                            mFrameListModel->frames()));
    mApplyingChanges = false;
}

void TileAnimationEditor::tileAnimationChanged(Tile *tile)
{
    if (mTile != tile)
        return;
    if (mApplyingChanges)
        return;

    mFrameListModel->setFrames(tile->tileset(), tile->frames());
}

void TileAnimationEditor::undo()
{
    if (mMapDocument)
        mMapDocument->undoStack()->undo();
}

void TileAnimationEditor::redo()
{
    if (mMapDocument)
        mMapDocument->undoStack()->redo();
}

void TileAnimationEditor::delete_()
{
    if (!mMapDocument)
        return;

    QItemSelectionModel *selectionModel = mUi->frameList->selectionModel();
    const QModelIndexList indexes = selectionModel->selectedIndexes();

    if (indexes.isEmpty())
        return;

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Delete Frames"));

    RangeSet<int> ranges;
    foreach (const QModelIndex &index, indexes)
        ranges.insert(index.row());

    // Iterate backwards over the ranges in order to keep the indexes valid
    RangeSet<int>::Range firstRange = ranges.begin();
    RangeSet<int>::Range it = ranges.end();
    if (it == firstRange) // no range
        return;

    do {
        --it;
        mFrameListModel->removeRows(it.first(), it.length(), QModelIndex());
    } while (it != firstRange);

    undoStack->endMacro();
}

} // namespace Internal
} // namespace Tiled

#include "tileanimationeditor.moc"
