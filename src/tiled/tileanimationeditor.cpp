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
#include "mapobject.h"
#include "rangeset.h"
#include "tile.h"
#include "tileanimationdriver.h"
#include "tiled.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "utils.h"
#include "zoomable.h"
#include "preferences.h"

#include <QAbstractListModel>
#include <QCloseEvent>
#include <QShortcut>
#include <QUndoStack>
#include <QMimeData>
#include <QSettings>

static const char * const FRAME_DURATION_KEY = "Animation/FrameDuration";

namespace Tiled {

class FrameListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FrameListModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
        , mTileset(nullptr)
    {
        // Restore previously used FrameDuration
        QSettings *s = Preferences::instance()->settings();
        mDefaultDuration = s->value(QLatin1String(FRAME_DURATION_KEY), 100).toInt();
    }

    int defaultDuration() const;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool removeRows(int row, int count, const QModelIndex &parent) override;

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column,
                      const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;

    void setFrames(const Tileset *tileset, const QVector<Frame> &frames);
    void addTileIdAsFrame(int id);
    void setDefaultFrameTime(int duration);
    const QVector<Frame> &frames() const;

private:
    int mDefaultDuration;

    void addFrame(const Frame &frame);

    const Tileset *mTileset;
    QVector<Frame> mFrames;
};

int FrameListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mFrames.size();
}

int FrameListModel::defaultDuration() const
{
    return mDefaultDuration;
}

QVariant FrameListModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return mFrames.at(index.row()).duration;
    case Qt::DecorationRole: {
        int tileId = mFrames.at(index.row()).tileId;
        if (Tile *tile = mTileset->findTile(tileId))
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
    if (indexes.isEmpty())
        return nullptr;

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
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
            frame.duration = mDefaultDuration;
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

void FrameListModel::addTileIdAsFrame(int id)
{
    Frame frame;
    frame.tileId = id;
    frame.duration = mDefaultDuration;
    addFrame(frame);
}

void FrameListModel::addFrame(const Frame &frame)
{
    beginInsertRows(QModelIndex(), mFrames.size(), mFrames.size());
    mFrames.append(frame);
    endInsertRows();
}

const QVector<Frame> &FrameListModel::frames() const
{
    return mFrames;
}

void FrameListModel::setDefaultFrameTime(int duration)
{
    mDefaultDuration = duration;
    Preferences::instance()->settings()->setValue(QLatin1String(FRAME_DURATION_KEY), duration);
}


TileAnimationEditor::TileAnimationEditor(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , mUi(new Ui::TileAnimationEditor)
    , mTilesetDocument(nullptr)
    , mTile(nullptr)
    , mFrameListModel(new FrameListModel(this))
    , mApplyingChanges(false)
    , mSuppressUndo(false)
    , mPreviewAnimationDriver(new TileAnimationDriver(this))
    , mPreviewFrameIndex(0)
    , mPreviewUnusedTime(0)
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));

    mUi->frameList->setModel(mFrameListModel);
    mUi->tilesetView->setMarkAnimatedTiles(false);
    mUi->tilesetView->zoomable()->setComboBox(mUi->zoomComboBox);
    mUi->frameTime->setValue(mFrameListModel->defaultDuration());

    connect(mUi->tilesetView, &QAbstractItemView::doubleClicked,
            this, &TileAnimationEditor::addFrameForTileAt);

    connect(mUi->tilesetView->zoomable(), &Zoomable::scaleChanged,
            this, &TileAnimationEditor::updatePreviewPixmap);

    connect(mFrameListModel, &QAbstractItemModel::dataChanged,
            this, &TileAnimationEditor::framesEdited);
    connect(mFrameListModel, &QAbstractItemModel::rowsInserted,
            this, &TileAnimationEditor::framesEdited);
    connect(mFrameListModel, &QAbstractItemModel::rowsRemoved,
            this, &TileAnimationEditor::framesEdited);
    connect(mFrameListModel, &QAbstractItemModel::rowsMoved,
            this, &TileAnimationEditor::framesEdited);

    connect(mPreviewAnimationDriver, &TileAnimationDriver::update,
            this, &TileAnimationEditor::advancePreviewAnimation);

    connect(mUi->frameTime, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &TileAnimationEditor::setDefaultFrameTime);

    connect(mUi->setFrameTimeButton, &QAbstractButton::clicked,
            this, &TileAnimationEditor::setFrameTime);

    QShortcut *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    QShortcut *redoShortcut = new QShortcut(QKeySequence::Redo, this);
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    QShortcut *deleteShortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);

    connect(undoShortcut, &QShortcut::activated, this, &TileAnimationEditor::undo);
    connect(redoShortcut, &QShortcut::activated, this, &TileAnimationEditor::redo);
    connect(deleteShortcut, &QShortcut::activated, this, &TileAnimationEditor::delete_);
    connect(deleteShortcut2, &QShortcut::activated, this, &TileAnimationEditor::delete_);

    Utils::restoreGeometry(this);

    mUi->horizontalSplitter->setSizes(QList<int>()
                                      << qRound(Utils::dpiScaled(128))
                                      << qRound(Utils::dpiScaled(512)));
}

TileAnimationEditor::~TileAnimationEditor()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void TileAnimationEditor::setTilesetDocument(TilesetDocument *tilesetDocument)
{
    if (mTilesetDocument)
        mTilesetDocument->disconnect(this);

    mTilesetDocument = tilesetDocument;
    mUi->tilesetView->setTilesetDocument(tilesetDocument);

    if (mTilesetDocument) {
        connect(mTilesetDocument, &TilesetDocument::tileAnimationChanged,
                this, &TileAnimationEditor::tileAnimationChanged);

        connect(mTilesetDocument, &TilesetDocument::currentObjectChanged,
                this, &TileAnimationEditor::currentObjectChanged);
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
        mFrameListModel->setFrames(nullptr, QVector<Frame>());
    }

    mUi->frameList->setEnabled(tile);

    resetPreview();
}

void TileAnimationEditor::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}

void TileAnimationEditor::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        mUi->retranslateUi(this);
        break;
    default:
        break;
    }
}

void TileAnimationEditor::showEvent(QShowEvent *)
{
    mPreviewAnimationDriver->start();
}

void TileAnimationEditor::hideEvent(QHideEvent *)
{
    mPreviewAnimationDriver->stop();
}

void TileAnimationEditor::framesEdited()
{

    if (mSuppressUndo)
        return;

    QUndoStack *undoStack = mTilesetDocument->undoStack();

    mApplyingChanges = true;
    undoStack->push(new ChangeTileAnimation(mTilesetDocument,
                                            mTile,
                                            mFrameListModel->frames()));
    mApplyingChanges = false;
}

void TileAnimationEditor::setDefaultFrameTime(int duration)
{
    mFrameListModel->setDefaultFrameTime(duration);
}

void TileAnimationEditor::setFrameTime()
{
    QItemSelectionModel *selectionModel = mUi->frameList->selectionModel();
    const QModelIndexList indexes = selectionModel->selectedIndexes();

    if (indexes.isEmpty())
        return;

    mSuppressUndo = true;

    for (const QModelIndex &index : indexes)
        mFrameListModel->setData(index, mUi->frameTime->value(), Qt::EditRole);

    mSuppressUndo = false;

    framesEdited();
}

void TileAnimationEditor::tileAnimationChanged(Tile *tile)
{
    if (mTile != tile)
        return;

    resetPreview();

    if (mApplyingChanges)
        return;

    mFrameListModel->setFrames(tile->tileset(), tile->frames());
}

void TileAnimationEditor::currentObjectChanged(Object *object)
{
    // If a tile object is selected, edit the animation frames for that tile
    if (object && object->typeId() == Object::MapObjectType) {
        const Cell &cell = static_cast<MapObject*>(object)->cell();
        if (Tile *tile = cell.tile())
            setTile(tile);
    }
}

void TileAnimationEditor::addFrameForTileAt(const QModelIndex &index)
{
    Q_ASSERT(mTile);

    const Tile *tile = mUi->tilesetView->tilesetModel()->tileAt(index);
    mFrameListModel->addTileIdAsFrame(tile->id());
}

void TileAnimationEditor::undo()
{
    if (mTilesetDocument)
        mTilesetDocument->undoStack()->undo();
}

void TileAnimationEditor::redo()
{
    if (mTilesetDocument)
        mTilesetDocument->undoStack()->redo();
}

void TileAnimationEditor::delete_()
{
    if (!mTilesetDocument || !mTile)
        return;

    QItemSelectionModel *selectionModel = mUi->frameList->selectionModel();
    const QModelIndexList indexes = selectionModel->selectedIndexes();

    if (indexes.isEmpty())
        return;

    QUndoStack *undoStack = mTilesetDocument->undoStack();
    undoStack->beginMacro(tr("Delete Frames"));

    RangeSet<int> ranges;
    for (const QModelIndex &index : indexes)
        ranges.insert(index.row());

    // Iterate backwards over the ranges in order to keep the indexes valid
    RangeSet<int>::Range firstRange = ranges.begin();
    RangeSet<int>::Range it = ranges.end();
    Q_ASSERT(it != firstRange); // no range not possible

    do {
        --it;
        mFrameListModel->removeRows(it.first(), it.length(), QModelIndex());
    } while (it != firstRange);

    undoStack->endMacro();
}

void TileAnimationEditor::advancePreviewAnimation(int ms)
{
    if (!mTile || !mTile->isAnimated())
        return;

    mPreviewUnusedTime += ms;

    const QVector<Frame> &frames = mTile->frames();
    Frame frame = frames.at(mPreviewFrameIndex);
    const int previousTileId = frame.tileId;

    while (frame.duration > 0 && mPreviewUnusedTime > frame.duration) {
        mPreviewUnusedTime -= frame.duration;
        mPreviewFrameIndex = (mPreviewFrameIndex + 1) % frames.size();

        frame = frames.at(mPreviewFrameIndex);
    }

    if (previousTileId != frame.tileId)
        updatePreviewPixmap();
}

void TileAnimationEditor::resetPreview()
{
    mPreviewFrameIndex = 0;
    mPreviewUnusedTime = 0;

    if (updatePreviewPixmap())
        return;

    mUi->preview->setText(QApplication::translate("TileAnimationEditor",
                                                  "Preview"));
}

bool TileAnimationEditor::updatePreviewPixmap()
{
    if (!mTile || !mTile->isAnimated())
        return false;

    const QVector<Frame> &frames = mTile->frames();
    const Tileset *tileset = mTile->tileset();
    const Frame frame = frames.at(mPreviewFrameIndex);

    if (Tile *tile = tileset->findTile(frame.tileId)) {
        const QPixmap &image = tile->image();
        const qreal scale = mUi->tilesetView->zoomable()->scale();

        const int w = image.width() * scale;
        const int h = image.height() * scale;
        mUi->preview->setPixmap(image.scaled(w, h, Qt::KeepAspectRatio));
        return true;
    }

    return false;
}

} // namespace Tiled

#include "tileanimationeditor.moc"
