/*
 * tilesetview.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilesetview.h"

#include "actionmanager.h"
#include "addremovetiles.h"
#include "changetile.h"
#include "changeevents.h"
#include "changetilewangid.h"
#include "pannableviewhelper.h"
#include "preferences.h"
#include "stylehelper.h"
#include "tile.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetmodel.h"
#include "utils.h"
#include "wangoverlay.h"
#include "zoomable.h"

#include <QAbstractItemDelegate>
#include <QApplication>
#include <QCoreApplication>
#include <QGesture>
#include <QGestureEvent>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPinchGesture>
#include <QScrollBar>
#include <QUndoCommand>
#include <QWheelEvent>
#include <QtCore/qmath.h>

#include <QDebug>

using namespace Tiled;

namespace {

static void setupTilesetGridTransform(const Tileset &tileset, QTransform &transform, QRect &targetRect)
{
    if (tileset.orientation() == Tileset::Isometric) {
        const QPoint tileCenter = targetRect.center();
        targetRect.setHeight(targetRect.width());
        targetRect.moveCenter(tileCenter);

        const QSize gridSize = tileset.gridSize();

        transform.translate(tileCenter.x(), tileCenter.y());

        const auto ratio = (qreal) gridSize.height() / gridSize.width();
        const auto scaleX = 1.0 / sqrt(2.0);
        const auto scaleY = scaleX * ratio;
        transform.scale(scaleX, scaleY);

        transform.rotate(45.0);

        transform.translate(-tileCenter.x(), -tileCenter.y());
    }
}

/**
 * The delegate for drawing tile items in the tileset view.
 */
class TileDelegate : public QAbstractItemDelegate
{
public:
    TileDelegate(TilesetView *tilesetView, QObject *parent = nullptr)
        : QAbstractItemDelegate(parent)
        , mTilesetView(tilesetView)
    { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void paintTile(QPainter *painter, const TilesetModel *model, Tile *tile,
                   QRect targetRect, QBrush highlight, bool selected, bool hovered) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    static void drawFilmStrip(QPainter *painter, QRect targetRect);

    void drawWangOverlay(QPainter *painter,
                         const Tile *tile,
                         QRect targetRect,
                         bool hovered) const;

    TilesetView *mTilesetView;
};

void TileDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    const TilesetModel *model = static_cast<const TilesetModel*>(index.model());
    if (!model)
        return;

    if (model->isFixedAtlas())
        return;

    Tile *tile = model->tileAt(index);
    if (!tile)
        return;

    paintTile(painter,
            model,
            tile,
            option.rect,
            option.palette.highlight(),
            option.state & QStyle::State_Selected,
            mTilesetView->hoveredIndex() == index);
}

void TileDelegate::paintTile(QPainter *painter,
                             const TilesetModel *model,
                             Tile *tile,
                             QRect targetRect,
                             QBrush highlight,
                             bool selected,
                             bool hovered) const
{
    if (!tile)
        return;

    const QPixmap &tileImage = tile->image();
    const int extra = mTilesetView->drawGrid() ? 1 : 0;
    const qreal zoom = mTilesetView->scale();
    const bool wrapping = mTilesetView->dynamicWrapping();

    QSize tileSize = tile->size();
    if (tileImage.isNull()) {
        Tileset *tileset = model->tileset();
        if (tileset->isCollection()) {
            tileSize = QSize(32, 32);
        } else {
            int max = std::max(tileset->tileWidth(), tileset->tileHeight());
            int min = std::min(max, 32);
            tileSize = QSize(min, min);
        }
    }

    // Compute rectangle to draw the image in: bottom- and left-aligned
    targetRect = targetRect.adjusted(0, 0, -extra, -extra);

    if (wrapping) {
        qreal scale = std::min(static_cast<qreal>(targetRect.width()) / tileSize.width(),
                               static_cast<qreal>(targetRect.height()) / tileSize.height());
        tileSize *= scale;

        auto center = targetRect.center();
        targetRect.setSize(tileSize);
        targetRect.moveCenter(center);
    } else {
        tileSize *= zoom;
        targetRect.setTop(targetRect.bottom() - tileSize.height() + 1);
        targetRect.setRight(targetRect.left() + tileSize.width() - 1);
    }

    // Draw the tile image
    if (Zoomable *zoomable = mTilesetView->zoomable())
        if (zoomable->smoothTransform())
            painter->setRenderHint(QPainter::SmoothPixmapTransform);

    if (!tileImage.isNull())
        painter->drawPixmap(targetRect, tileImage, tile->imageRect());
    else
        mTilesetView->imageMissingIcon().paint(painter, targetRect, Qt::AlignBottom | Qt::AlignLeft);


    // Overlay with film strip when animated
    if (mTilesetView->markAnimatedTiles() && tile->isAnimated())
        drawFilmStrip(painter, targetRect);

    // Overlay with highlight color when selected
    if (selected) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.5);
        painter->fillRect(targetRect, highlight);
        painter->setOpacity(opacity);
    }

    if (mTilesetView->isEditWangSet())
        drawWangOverlay(painter, tile, targetRect, hovered);
}

QSize TileDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                             const QModelIndex &index) const
{
    const TilesetModel *m = static_cast<const TilesetModel*>(index.model());
    const int extra = mTilesetView->drawGrid() ? 1 : 0;
    const qreal scale = mTilesetView->scale();
    Tileset *tileset = m->tileset();

    if (m->isFixedAtlas())
        return QSize(tileset->imageWidth() * scale + extra,
                     tileset->imageHeight() * scale + extra);

    if (const Tile *tile = m->tileAt(index)) {
        if (mTilesetView->dynamicWrapping())
            return QSize(mTilesetView->maxTileWidth() * scale + extra,
                         mTilesetView->maxTileHeight() * scale + extra);

        QSize tileSize = tile->size();

        if (tile->image().isNull()) {
            if (tileset->isCollection()) {
                tileSize = QSize(32, 32);
            } else {
                int max = std::max(tileset->tileWidth(), tileset->tileWidth());
                int min = std::min(max, 32);
                tileSize = QSize(min, min);
            }
        }

        return QSize(tileSize.width() * scale + extra,
                     tileSize.height() * scale + extra);
    }

    return QSize(extra, extra);
}

void TileDelegate::drawFilmStrip(QPainter *painter, QRect targetRect)
{
    painter->save();

    qreal scale = qMin(targetRect.width() / 32.0,
                       targetRect.height() / 32.0);

    painter->setClipRect(targetRect);
    painter->translate(targetRect.right(),
                       targetRect.bottom());
    painter->scale(scale, scale);
    painter->translate(-18, 3);
    painter->rotate(-45);
    painter->setOpacity(0.8);

    QRectF strip(0, 0, 32, 6);
    painter->fillRect(strip, Qt::black);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);

    QRectF hole(0, 0, strip.height() * 0.6, strip.height() * 0.6);
    qreal step = (strip.height() - hole.height()) + hole.width();
    qreal margin = (strip.height() - hole.height()) / 2;

    for (qreal x = (step - hole.width()) / 2; x < strip.right(); x += step) {
        hole.moveTo(x, margin);
        painter->drawRoundedRect(hole, 25, 25, Qt::RelativeSize);
    }

    painter->restore();
}

void TileDelegate::drawWangOverlay(QPainter *painter,
                                   const Tile *tile,
                                   QRect targetRect,
                                   bool hovered) const
{
    WangSet *wangSet = mTilesetView->wangSet();
    if (!wangSet)
        return;

    painter->save();

    QTransform transform;
    setupTilesetGridTransform(*tile->tileset(), transform, targetRect);
    painter->setTransform(transform, true);

    paintWangOverlay(painter,
                     wangSet->wangIdOfTile(tile) & wangSet->typeMask(),
                     *wangSet,
                     targetRect);

    if (hovered) {
        qreal opacity = painter->opacity();
        painter->setOpacity(0.5);
        paintWangOverlay(painter, mTilesetView->wangId(),
                         *wangSet,
                         targetRect,
                         WangOverlayOptions());
        painter->setOpacity(opacity);
    }

    painter->restore();
}

} // anonymous namespace

TilesetView::TilesetView(QWidget *parent)
    : QTableView(parent)
    , mZoomable(new Zoomable(this))
    , mRubberBand(QRubberBand::Rectangle, this)
    , mImageMissingIcon(QStringLiteral("://images/32/image-missing.png"))
{
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setItemDelegate(new TileDelegate(this, this));
    setShowGrid(false);
    setTabKeyNavigation(false);
    setDropIndicatorShown(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHeaderView *hHeader = horizontalHeader();
    QHeaderView *vHeader = verticalHeader();
    hHeader->hide();
    vHeader->hide();
    hHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    vHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    hHeader->setMinimumSectionSize(1);
    vHeader->setMinimumSectionSize(1);
#if QT_VERSION == QT_VERSION_CHECK(6, 9, 0)
    // For Qt 6.9.0 we force header mode to "FlexibleWithSectionMemoryUsage",
    // to avoid what appears to be a bug in that implementation regarding
    // resizing of sections (https://github.com/mapeditor/tiled/issues/4191).
    hHeader->setStretchLastSection(true);
    vHeader->setStretchLastSection(true);
    hHeader->setStretchLastSection(false);
    vHeader->setStretchLastSection(false);
#endif

    // Hardcode this view on 'left to right' since it doesn't work properly
    // for 'right to left' languages.
    setLayoutDirection(Qt::LeftToRight);

    Preferences *prefs = Preferences::instance();
    mDrawGrid = prefs->showTilesetGrid();

    grabGesture(Qt::PinchGesture);

    connect(prefs, &Preferences::showTilesetGridChanged,
            this, &TilesetView::setDrawGrid);

    connect(StyleHelper::instance(), &StyleHelper::styleApplied,
            this, &TilesetView::updateBackgroundColor);

    connect(mZoomable, &Zoomable::scaleChanged, this, &TilesetView::adjustScale);

    connect(new PannableViewHelper(this), &PannableViewHelper::cursorChanged,
            this, [this] (std::optional<Qt::CursorShape> cursor) {
        if (cursor)
            viewport()->setCursor(*cursor);
        else
            viewport()->unsetCursor();
    });
}

void TilesetView::setTilesetDocument(TilesetDocument *tilesetDocument)
{
    if (mTilesetDocument)
        mTilesetDocument->disconnect(this);

    mTilesetDocument = tilesetDocument;

    if (mTilesetDocument) {
        connect(mTilesetDocument, &Document::changed, this, &TilesetView::onChange);
        connect(mTilesetDocument, &TilesetDocument::tilesAdded, this, &TilesetView::refreshColumnCount);
        connect(mTilesetDocument, &TilesetDocument::tilesRemoved, this, &TilesetView::refreshColumnCount);
        connect(mTilesetDocument, &TilesetDocument::tileImageSourceChanged, this, &TilesetView::refreshColumnCount);
    }
}

QSize TilesetView::sizeHint() const
{
    return Utils::dpiScaled(QSize(260, 100));
}

int TilesetView::sizeHintForColumn(int column) const
{
    Q_UNUSED(column)
    const TilesetModel *model = tilesetModel();
    if (!model)
        return -1;
    if (model->tileset()->isCollection() || model->tileset()->isAtlas())
        return QTableView::sizeHintForColumn(column);

    const int gridSpace = mDrawGrid ? 1 : 0;
    if (dynamicWrapping())
        return model->tileset()->tileWidth() * scale() + gridSpace;

    const int tileWidth = model->tileset()->tileWidth();
    return qRound(tileWidth * scale()) + gridSpace;
}

int TilesetView::sizeHintForRow(int row) const
{
    Q_UNUSED(row)
    const TilesetModel *model = tilesetModel();
    if (!model)
        return -1;
    if (model->tileset()->isCollection() || model->tileset()->isAtlas())
        return QTableView::sizeHintForRow(row);

    const int gridSpace = mDrawGrid ? 1 : 0;
    if (dynamicWrapping())
        return model->tileset()->tileHeight() * scale() + gridSpace;

    const int tileHeight = model->tileset()->tileHeight();
    return qRound(tileHeight * scale()) + gridSpace;
}

qreal TilesetView::scale() const
{
    return mZoomable->scale();
}

void TilesetView::setDynamicWrapping(bool enabled)
{
    WrapBehavior behavior = enabled ? WrapDynamic : WrapFixed;
    if (mWrapBehavior == behavior)
        return;

    mWrapBehavior = behavior;
    setVerticalScrollBarPolicy(dynamicWrapping() ? Qt::ScrollBarAlwaysOn
                                                 : Qt::ScrollBarAsNeeded);
    scheduleDelayedItemsLayout();
    refreshColumnCount();
}

bool TilesetView::dynamicWrapping() const
{
    switch (mWrapBehavior) {
    case WrapDefault:
        if (tilesetModel())
            return tilesetModel()->tileset()->isCollection();
        break;
    case WrapDynamic:
        if (tilesetModel() && tilesetModel()->tileset()->isAtlas())
            return !mRelocateTiles;
        return true;
    case WrapFixed:
        return false;
    }

    return false;
}

void TilesetView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    updateBackgroundColor();
    setVerticalScrollBarPolicy(dynamicWrapping() ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAsNeeded);
    refreshColumnCount();
}

void TilesetView::setMarkAnimatedTiles(bool enabled)
{
    if (mMarkAnimatedTiles == enabled)
        return;

    mMarkAnimatedTiles = enabled;
    viewport()->update();
}

bool TilesetView::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
        if (QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture))
            mZoomable->handlePinchGesture(static_cast<QPinchGesture *>(gesture));
    } else if (event->type() == QEvent::ShortcutOverride) {
        auto keyEvent = static_cast<QKeyEvent*>(event);
        if (Utils::isZoomInShortcut(keyEvent) ||
                Utils::isZoomOutShortcut(keyEvent) ||
                Utils::isResetZoomShortcut(keyEvent)) {
            event->accept();
            return true;
        }
    }

    return QTableView::event(event);
}

void TilesetView::keyPressEvent(QKeyEvent *event)
{
    if (Utils::isZoomInShortcut(event)) {
        mZoomable->zoomIn();
        return;
    }
    if (Utils::isZoomOutShortcut(event)) {
        mZoomable->zoomOut();
        return;
    }
    if (Utils::isResetZoomShortcut(event)) {
        mZoomable->resetZoom();
        return;
    }

    // TODO: These shortcuts only work while the TilesetView is focused. It
    // would be preferable if they could be used more globally.
    if (mEditWangSet && mWangBehavior == AssignWholeId && !(event->modifiers() & Qt::ControlModifier)) {
        WangId transformedWangId = mWangId;

        if (event->key() == Qt::Key_Z) {
            if (event->modifiers() & Qt::ShiftModifier)
                transformedWangId.rotate(-1);
            else
                transformedWangId.rotate(1);
        } else if (event->key() == Qt::Key_X) {
            transformedWangId.flipHorizontally();
        } else if (event->key() == Qt::Key_Y) {
            transformedWangId.flipVertically();
        }

        if (mWangId != transformedWangId) {
            setWangId(transformedWangId);
            emit currentWangIdChanged(mWangId);
            return;
        }
    }

    // Ignore space, because we'd like to use it for panning
    if (event->key() == Qt::Key_Space) {
        event->ignore();
        return;
    }

    return QTableView::keyPressEvent(event);
}

void TilesetView::setRelocateTiles(bool enabled)
{
    if (mRelocateTiles == enabled)
        return;

    mRelocateTiles = enabled;

    if (TilesetModel *m = tilesetModel(); m && m->tileset()->isAtlas()) {
        selectionModel()->clear();
        enabled = false;
    }

    if (enabled)
        setDragDropMode(QTableView::InternalMove);
    else
        setDragDropMode(QTableView::NoDragDrop);

    refreshColumnCount();
    setMouseTracking(true);
    viewport()->update();
}

void TilesetView::setEditWangSet(bool enabled)
{
    if (mEditWangSet == enabled)
        return;

    mEditWangSet = enabled;
    setMouseTracking(true);
    viewport()->update();
}

void TilesetView::setWangSet(WangSet *wangSet)
{
    if (mWangSet == wangSet)
        return;

    mWangSet = wangSet;

    if (mEditWangSet)
        viewport()->update();
}

/**
 * Sets the WangId and changes WangBehavior to WholeId.
 */
void TilesetView::setWangId(WangId wangId)
{
    mWangId = wangId;
    mWangBehavior = AssignWholeId;

    if (mEditWangSet && mHoveredIndex.isValid()) {
        if (TilesetModel *m = tilesetModel(); m && m->isFixedAtlas()) {
            viewport()->update();
        } else {
            update(mHoveredIndex);
        }
    }
}

/**
 * Sets the wangColor, and changes WangBehavior depending on the type of the
 * WangSet.
 */
void TilesetView::setWangColor(int color)
{
    mWangColorIndex = color;
    mWangBehavior = AssignHoveredIndex;
}

QIcon TilesetView::imageMissingIcon() const
{
    return QIcon::fromTheme(QLatin1String("image-missing"), mImageMissingIcon);
}

void TilesetView::mousePressEvent(QMouseEvent *event)
{
    if (mEditWangSet) {
        if (event->button() == Qt::LeftButton)
            applyWangId();
        return;
    }

    const TilesetModel *model = tilesetModel();
    if (mRelocateTiles && model && model->tileset()->isAtlas()) {
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
            mSnapToGrid = !(event->modifiers() & Qt::ShiftModifier);

            if (event->button() == Qt::LeftButton) {
                mDraggedIndex = indexAt(event->pos());
                Tile* tile = model->tileAt(mDraggedIndex);
                if (mDraggedIndex.isValid() && tile) {
                    QRect tileRect = tileToView(tile->imageRect());
                    const int edge = 4 * scale();
                    if (qAbs(event->pos().x() - tileRect.right()) < edge)
                        mResizingEdge = 1;
                    else if (qAbs(event->pos().y() - tileRect.bottom()) < edge)
                        mResizingEdge = 2;
                    else if (qAbs(event->pos().x() - tileRect.left()) < edge)
                        mResizingEdge = 3;
                    else if (qAbs(event->pos().y() - tileRect.top()) < edge)
                        mResizingEdge = 4;
                    else
                        mResizingEdge = 0;

                    mSelectionOffset = viewToTile(event->pos()) - tile->imageRect().topLeft();
                    mRubberBand.setGeometry(tileRect);
                    mRubberBand.show();
                    event->accept();
                    return;
                }
            }

            mAtlasSelecting = true;
            mSelectionOffset = viewToTile(event->pos());
            updateAtlasSelection(mSelectionOffset);
            event->accept();
            return;
        }
    }

    QTableView::mousePressEvent(event);
}

void TilesetView::mouseMoveEvent(QMouseEvent *event)
{
    if (mAtlasSelecting) {
        mSnapToGrid = !(event->modifiers() & Qt::ShiftModifier);
        updateAtlasSelection(viewToTile(event->pos()));
        event->accept();
        return;
    }

    const TilesetModel *model = tilesetModel();
    if (!model)
        return;

    const QModelIndex hoveredIndex = indexAt(event->pos());
    if (hoveredIndex != mHoveredIndex) {
        const QModelIndex previousHoveredIndex = mHoveredIndex;
        mHoveredIndex = hoveredIndex;

        if (model->tileset()->isAtlas()) {
            viewport()->update();
        } else {
            if (previousHoveredIndex.isValid())
                update(previousHoveredIndex);
            if (mHoveredIndex.isValid())
                update(mHoveredIndex);
        }
    }

    if (mDraggedIndex.isValid()) {
        mSnapToGrid = !(event->modifiers() & Qt::ShiftModifier);

        if (Tile *tile = model->tileAt(mDraggedIndex)) {
            QRect newRect = tile->imageRect();
            QPoint pos = viewToTile(event->pos());

            // Apply resize or move based on where we initially clicked
            switch (mResizingEdge) {
            case 1: // Right edge
                newRect.setRight(pos.x());
                break;
            case 2: // Bottom edge
                newRect.setBottom(pos.y());
                break;
            case 3: // Left edge
                newRect.setLeft(pos.x());
                break;
            case 4: // Top edge
                newRect.setTop(pos.y());
                break;
            default: // No edge - normal move
                pos -= mSelectionOffset;
                break;
            }

            if (mSnapToGrid) {
                const int margin = model->tileset()->margin();
                const int spacing = model->tileset()->tileSpacing();
                const int tileWidth = model->tileset()->tileWidth();
                const int tileHeight = model->tileset()->tileHeight();

                if (mResizingEdge == 0) {
                    pos.setX((qMax(pos.x() - margin, 0) / (tileWidth + spacing)) * (tileWidth + spacing) + margin);
                    pos.setY((qMax(pos.y() - margin, 0) / (tileHeight + spacing)) * (tileHeight + spacing) + margin);
                } else {
                    // Calculate position relative to first tile (after margin)
                    const int relLeft = qMax(newRect.left() - margin, 0);
                    const int relTop = qMax(newRect.top() - margin, 0);
                    const int relRight = qMax(newRect.right() - margin, 0);
                    const int relBottom = qMax(newRect.bottom() - margin, 0);

                    // Find closest tile positions
                    const int tileLeft = relLeft / (tileWidth + spacing);
                    const int tileTop = relTop / (tileHeight + spacing);
                    const int tileRight = (relRight + spacing) / (tileWidth + spacing);
                    const int tileBottom = (relBottom + spacing) / (tileHeight + spacing);

                    // Convert back to absolute coordinates with margin
                    newRect.setLeft(margin + tileLeft * (tileWidth + spacing));
                    newRect.setTop(margin + tileTop * (tileHeight + spacing));
                    newRect.setRight(margin + tileRight * (tileWidth + spacing) + tileWidth - 1);
                    newRect.setBottom(margin + tileBottom * (tileHeight + spacing) + tileHeight - 1);
                }
            }

            if (mResizingEdge == 0)
                newRect.moveTopLeft(pos);

            mRubberBand.setGeometry(tileToView(newRect));
        }

        event->accept();
        return;
    }

    if (!mEditWangSet) {
        QTableView::mouseMoveEvent(event);
        return;
    }

    if (!mWangSet)
        return;

    const QPoint pos = event->pos();
    WangId wangId;

    if (mWangBehavior == AssignWholeId) {
        wangId = mWangId;
    } else {
        QRect tileRect = visualRect(mHoveredIndex);
        QTransform transform;
        setupTilesetGridTransform(*tilesetDocument()->tileset(), transform, tileRect);

        if (!tileRect.isEmpty()) {
            const auto mappedPos = transform.inverted().map(pos);
            QPoint tileLocalPos = mappedPos - tileRect.topLeft();
            QPointF tileLocalPosF((qreal) tileLocalPos.x() / tileRect.width(),
                                  (qreal) tileLocalPos.y() / tileRect.height());

            const int x = qBound(0, qFloor(tileLocalPosF.x() * 3), 2);
            const int y = qBound(0, qFloor(tileLocalPosF.y() * 3), 2);
            WangId::Index index = WangId::indexByGrid(x, y);

            if (index != WangId::NumIndexes) {  // center is dead zone
                switch (mWangSet->type()) {
                case WangSet::Edge:
                    tileLocalPosF -= QPointF(0.5, 0.5);

                    if (tileLocalPosF.x() < tileLocalPosF.y()) {
                        if (tileLocalPosF.x() > -tileLocalPosF.y())
                            index = WangId::Bottom;
                        else
                            index = WangId::Left;
                    } else {
                        if (tileLocalPosF.x() > -tileLocalPosF.y())
                            index = WangId::Right;
                        else
                            index = WangId::Top;
                    }
                    break;
                case WangSet::Corner:
                    if (tileLocalPosF.x() > 0.5) {
                        if (tileLocalPosF.y() > 0.5)
                            index = WangId::BottomRight;
                        else
                            index = WangId::TopRight;
                    } else {
                        if (tileLocalPosF.y() > 0.5)
                            index = WangId::BottomLeft;
                        else
                            index = WangId::TopLeft;
                    }
                    break;
                case WangSet::Mixed:
                    break;
                }

                wangId.setIndexColor(index, mWangColorIndex ? mWangColorIndex
                                                            : WangId::INDEX_MASK);
            }
        }
    }

    if (wangId != mWangId) {
        mWangId = wangId;
        viewport()->update();
    }

    if (event->buttons() & Qt::LeftButton)
        applyWangId();
}

void TilesetView::mouseReleaseEvent(QMouseEvent *event)
{
    const TilesetModel *model = tilesetModel();
    if (!model)
        return;

    if (mDraggedIndex.isValid() && event->button() == Qt::LeftButton) {
        if (Tile* tile = model->tileAt(mDraggedIndex))
            mTilesetDocument->undoStack()->push(new ChangeTileImageRect(mTilesetDocument, { tile }, { viewToTile(mRubberBand.geometry()) }));

        mDraggedIndex = QModelIndex();
        mRubberBand.hide();
        event->accept();
        return;
    }

    if (mAtlasSelecting && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)) {
        QRect tileRect = mRubberBand.geometry().isEmpty() ? QRect(mSelectionOffset, QSize(1, 1)) : viewToTile(mRubberBand.geometry());

        if (event->button() == Qt::RightButton) {
            QList<Tile*> tiles;
            for (Tile *tile : tilesetModel()->tileset()->tiles()) {
                if (tileRect.intersects(tile->imageRect()))
                    tiles.append(tile);
            }
            mTilesetDocument->undoStack()->push(new RemoveTiles(mTilesetDocument, tiles));
        } else {
            Tile *newTile = new Tile(tilesetModel()->tileset()->takeNextTileId(), tilesetModel()->tileset());
            newTile->setImageRect(tileRect);
            mTilesetDocument->undoStack()->push(new AddTiles(mTilesetDocument, QList<Tile*>() << newTile));
        }

        mAtlasSelecting = false;
        mRubberBand.hide();
        event->accept();
        return;
    }

    if (mEditWangSet) {
        if (event->button() == Qt::LeftButton)
            finishWangIdChange();
        return;
    }

    QTableView::mouseReleaseEvent(event);
}

void TilesetView::paintEvent(QPaintEvent *event)
{
    QTableView::paintEvent(event);

    TilesetModel *model = tilesetModel();
    if (!model || !model->isFixedAtlas())
        return;

    QPainter painter(viewport());
    const Tileset *tileset = model->tileset();

    // Draw tileset background image when in relocate mode
    if (mRelocateTiles) {
        if (!tileset->imageSource().isEmpty()) {
            const QRect sourceRect(0, 0, tileset->imageWidth(), tileset->imageHeight());
            const QRect viewRect = tileToView(sourceRect);
            painter.setOpacity(0.5);
            painter.drawPixmap(viewRect, tileset->image(), sourceRect);
            painter.setOpacity(1.0);
        }
    }

    // Draw tiles
    TileDelegate *delegate = static_cast<TileDelegate*>(itemDelegate());
    QItemSelectionModel *s = selectionModel();

    for (Tile *tile : tileset->tiles()) {
        const QRect rect = tileToView(tile->imageRect());
        const QModelIndex index = model->tileIndex(tile);
        const bool selected = s->isSelected(index) || index == s->currentIndex();
        delegate->paintTile(&painter, model, tile, rect, palette().highlight(), selected, mHoveredIndex == index);

        // Draw grid
        if (mDrawGrid) {
            if (mRelocateTiles) {
                painter.setPen(palette().highlight().color());
            } else {
                painter.setPen(palette().base().color());
            }
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(rect);
        }

        // Draw resize indicators
        if (mRelocateTiles && (mDraggedIndex == index || (!mDraggedIndex.isValid() && mHoveredIndex == index))) {
            const int indicatorSize = 4 * scale();
            QColor indicatorColor = palette().highlight().color().lighter(150);
            indicatorColor.setAlpha(180);  // Make slightly transparent
            painter.setPen(Qt::NoPen);
            painter.setBrush(indicatorColor);

            // Top-left corner
            painter.drawRect(QRect(rect.topLeft(), QSize(indicatorSize, indicatorSize)));

            // Top-right corner
            painter.drawRect(QRect(rect.topRight() - QPoint(indicatorSize, 0), QSize(indicatorSize, indicatorSize)));

            // Bottom-left corner
            painter.drawRect(QRect(rect.bottomLeft() - QPoint(0, indicatorSize), QSize(indicatorSize, indicatorSize)));

            // Bottom-right corner
            painter.drawRect(QRect(rect.bottomRight() - QPoint(indicatorSize, indicatorSize), QSize(indicatorSize, indicatorSize)));

            // Reset painter state
            painter.setPen(palette().base().color());
            painter.setBrush(Qt::NoBrush);
        }
    }
}

QModelIndex TilesetView::indexAt(const QPoint &pos) const
{
    if (TilesetModel *m = tilesetModel(); m && m->isFixedAtlas()) {
        const QPoint tilesetPos = viewToTile(pos);
        for (Tile *tile : m->tileset()->tiles())
            if (tile->imageRect().contains(tilesetPos))
                return tilesetModel()->tileIndex(tile);
        return QModelIndex();
    }

    return QTableView::indexAt(pos);
}

void TilesetView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    if (TilesetModel *m = tilesetModel(); m && m->isFixedAtlas()) {
        return;
    }

    QTableView::scrollTo(index, hint);
}

void TilesetView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    const TilesetModel *model = tilesetModel();

    if (!model || !model->isFixedAtlas()) {
        QTableView::setSelection(rect, flags);
        return;
    }

    // Select tiles based on position in the rect
    const QRect tilesetRect = viewToTile(rect);
    QItemSelection selection;
    for (Tile *tile : model->tileset()->tiles()) {
        if (tile->imageRect().intersects(tilesetRect)) {
            const QModelIndex index = model->tileIndex(tile);
            selection.select(index, index);
        }
    }
    selectionModel()->select(selection, flags);
}

void TilesetView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected, deselected);

    if (TilesetModel *m = tilesetModel(); m && m->isFixedAtlas())
        viewport()->update();
}

QRect TilesetView::visualRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    if (TilesetModel *m = tilesetModel(); m && m->isFixedAtlas()) {
        if (Tile *tile = m->tileAt(index))
            return tileToView(tile->imageRect());
        return QRect();
    }

    return QTableView::visualRect(index);
}

void TilesetView::leaveEvent(QEvent *event)
{
    if (mHoveredIndex.isValid()) {
        const QModelIndex previousHoveredIndex = mHoveredIndex;
        mHoveredIndex = QModelIndex();
        if (TilesetModel *m = tilesetModel(); m && m->isFixedAtlas()) {
            viewport()->update();
        } else {
            update(previousHoveredIndex);
        }
    }

    if (mDraggedIndex.isValid()) {
        mDraggedIndex = QModelIndex();
        mRubberBand.hide();
    }

    QTableView::leaveEvent(event);
}

/**
 * Override to support zooming in and out using the mouse wheel, as well as to
 * make the scrolling speed independent of Ctrl modifier and zoom level.
 */
void TilesetView::wheelEvent(QWheelEvent *event)
{
    auto hor = horizontalScrollBar();
    auto ver = verticalScrollBar();

    bool wheelZoomsByDefault = !dynamicWrapping() && Preferences::instance()->wheelZoomsByDefault();
    bool control = event->modifiers() & Qt::ControlModifier;

    if ((wheelZoomsByDefault != control) && event->angleDelta().y()) {

        const QPointF &viewportPos = event->position();
        const QPointF contentPos(viewportPos.x() + hor->value(),
                                 viewportPos.y() + ver->value());

        QPointF relativeContentPos;

        const QSize oldContentSize = viewportSizeHint();
        if (!oldContentSize.isEmpty()) {
            relativeContentPos = QPointF(contentPos.x() / oldContentSize.width(),
                                         contentPos.y() / oldContentSize.height());
        }

        mZoomable->handleWheelDelta(event->angleDelta().y());

        executeDelayedItemsLayout();

        const QSize newContentSizeHint = viewportSizeHint();
        const QPointF newContentPos(relativeContentPos.x() * newContentSizeHint.width(),
                                    relativeContentPos.y() * newContentSizeHint.height());

        hor->setValue(newContentPos.x() - viewportPos.x());
        ver->setValue(newContentPos.y() - viewportPos.y());
        return;
    }

    QPoint delta = event->pixelDelta();
    if (delta.isNull())
        delta = Utils::dpiScaled(event->angleDelta());

    if (delta.x())
        hor->setValue(hor->value() - delta.x());
    if (delta.y())
        ver->setValue(ver->value() - delta.y());
}

/**
 * Allow changing tile properties through a context menu.
 */
void TilesetView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    const TilesetModel *model = tilesetModel();
    if (!model)
        return;

    if (mRelocateTiles && model->tileset()->isAtlas()) {
        return;
    }

    Tile *tile = model->tileAt(index);

    QMenu menu;

    if (tile) {
        if (mEditWangSet) {
            selectionModel()->setCurrentIndex(index,
                                              QItemSelectionModel::SelectCurrent |
                                              QItemSelectionModel::Clear);

            if (mWangSet) {
                QAction *setImage = menu.addAction(tr("Use as Terrain Set Image"));
                connect(setImage, &QAction::triggered, this, &TilesetView::selectWangSetImage);
            }
            if (mWangBehavior != AssignWholeId && mWangColorIndex) {
                QAction *setImage = menu.addAction(tr("Use as Terrain Image"));
                connect(setImage, &QAction::triggered, this, &TilesetView::selectWangColorImage);
            }
            menu.addSeparator();
        }

        QUrl imageSource = tile->imageSource();
        if (imageSource.isEmpty())
            imageSource = tile->tileset()->imageSource();

        if (!imageSource.isEmpty()) {
            const QString localFile = imageSource.toLocalFile();
            if (!localFile.isEmpty()) {
                Utils::addOpenContainingFolderAction(menu, localFile);
                Utils::addOpenWithSystemEditorAction(menu, localFile);
                menu.addSeparator();
            }
        }

        if (mTilesetDocument) {
            const QIcon propIcon(QStringLiteral(":images/16/document-properties.png"));
            QAction *tileProperties = menu.addAction(propIcon,
                                                     tr("Tile &Properties..."));
            Utils::setThemeIcon(tileProperties, "document-properties");
            connect(tileProperties, &QAction::triggered, this, &TilesetView::editTileProperties);
        } else {
            // Assuming we're used in the MapEditor

            // Enable "swap" if there are exactly 2 tiles selected
            const bool exactlyTwoTilesSelected =
                    (selectionModel()->selectedIndexes().size() == 2);

            QAction *swapTilesAction = menu.addAction(tr("&Swap Tiles"));
            swapTilesAction->setEnabled(exactlyTwoTilesSelected);
            connect(swapTilesAction, &QAction::triggered, this, &TilesetView::swapTiles);
        }

        menu.addSeparator();
    }

    QAction *toggleGrid = menu.addAction(tr("Show &Grid"));
    toggleGrid->setCheckable(true);
    toggleGrid->setChecked(mDrawGrid);

    Preferences *prefs = Preferences::instance();
    connect(toggleGrid, &QAction::toggled,
            prefs, &Preferences::setShowTilesetGrid);

    QAction *selectAllTiles = menu.addAction(tr("Select &All Tiles"));
    connect(selectAllTiles, &QAction::triggered, this, &QAbstractItemView::selectAll);

    ActionManager::applyMenuExtensions(&menu, MenuIds::tilesetViewTiles);

    menu.exec(event->globalPos());
}

void TilesetView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    refreshColumnCount();
}

void TilesetView::onChange(const ChangeEvent &change)
{
    switch (change.type) {
    case ChangeEvent::DocumentReloaded:
        refreshColumnCount();
        break;
    case ChangeEvent::WangSetChanged: {
        auto &wangSetChange = static_cast<const WangSetChangeEvent&>(change);
        if (mEditWangSet && wangSetChange.wangSet == mWangSet &&
                (wangSetChange.property == WangSetChangeEvent::TypeProperty)) {
            viewport()->update();
        }
        break;
    }
    default:
        break;
    }
}

void TilesetView::selectWangSetImage()
{
    if (Tile *tile = currentTile())
        emit wangSetImageSelected(tile);
}

void TilesetView::selectWangColorImage()
{
    if (Tile *tile = currentTile())
        emit wangColorImageSelected(tile, mWangColorIndex);
}

void TilesetView::editTileProperties()
{
    Q_ASSERT(mTilesetDocument);

    Tile *tile = currentTile();
    if (!tile)
        return;

    mTilesetDocument->setCurrentObject(tile);
    emit mTilesetDocument->editCurrentObject();
}

void TilesetView::swapTiles()
{
    const QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    if (selectedIndexes.size() != 2)
        return;

    const TilesetModel *model = tilesetModel();
    Tile *tile1 = model->tileAt(selectedIndexes[0]);
    Tile *tile2 = model->tileAt(selectedIndexes[1]);

    if (!tile1 || !tile2)
        return;

    emit swapTilesRequested(tile1, tile2);
}

void TilesetView::setDrawGrid(bool drawGrid)
{
    mDrawGrid = drawGrid;
    scheduleDelayedItemsLayout();
    refreshColumnCount();
}

void TilesetView::adjustScale()
{
    scheduleDelayedItemsLayout();
    refreshColumnCount();
}

void TilesetView::refreshColumnCount()
{
    const TilesetModel *model = tilesetModel();
    if (!model)
        return;

    if (!dynamicWrapping()) {
        tilesetModel()->setColumnCountOverride(0);
        if (model->tileset()->isAtlas())
            viewport()->update();
        return;
    }

    const QSize maxSize = maximumViewportSize();
    const int gridSpace = mDrawGrid ? 1 : 0;
    if (model->tileset()->isAtlas()) {
        mMaxTileWidth = 0;
        mMaxTileHeight = 0;
        for (Tile *tile : model->tileset()->tiles()) {
            mMaxTileWidth = std::max(mMaxTileWidth, tile->imageRect().width());
            mMaxTileHeight = std::max(mMaxTileHeight, tile->imageRect().height());
        }
    } else {
        mMaxTileWidth = model->tileset()->tileWidth();
        mMaxTileHeight = model->tileset()->tileHeight();
    }
    const int scaledTileSize = std::max<int>(mMaxTileWidth * scale(), 1) + gridSpace;
    const int columnCount = std::max(maxSize.width() / scaledTileSize, 1);
    tilesetModel()->setColumnCountOverride(columnCount);
}

void TilesetView::applyWangId()
{
    if (!mHoveredIndex.isValid() || !mWangSet)
        return;

    Tile *tile = tilesetModel()->tileAt(mHoveredIndex);
    if (!tile)
        return;

    WangId previousWangId = mWangSet->wangIdOfTile(tile);
    WangId newWangId = previousWangId;

    if (mWangBehavior == AssignWholeId) {
        newWangId = mWangId;
    } else {
        for (int i = 0; i < WangId::NumIndexes; ++i) {
            if (mWangId.indexColor(i))
                newWangId.setIndexColor(i, mWangColorIndex);
        }
    }

    if (newWangId == previousWangId)
        return;

    bool wasUnused = !mWangSet->wangIdIsUsed(newWangId);

    QUndoCommand *command = new ChangeTileWangId(mTilesetDocument, mWangSet, tile, newWangId);
    mTilesetDocument->undoStack()->push(command);
    mWangIdChanged = true;

    if (!mWangSet->wangIdIsUsed(previousWangId))
        emit wangIdUsedChanged(previousWangId);

    if (wasUnused)
        emit wangIdUsedChanged(newWangId);
}

void TilesetView::finishWangIdChange()
{
    if (!mWangIdChanged)
        return;

    mTilesetDocument->undoStack()->push(new ChangeTileWangId);
    mWangIdChanged = false;
}

Tile *TilesetView::currentTile() const
{
    const TilesetModel *model = tilesetModel();
    return model ? model->tileAt(currentIndex()) : nullptr;
}

void TilesetView::updateBackgroundColor()
{
    QColor base = QApplication::palette().dark().color();

    if (TilesetModel *model = tilesetModel()) {
        Tileset *tileset = model->tileset();
        if (tileset->backgroundColor().isValid())
            base = tileset->backgroundColor();
    }

    QPalette p = palette();
    p.setColor(QPalette::Base, base);
    setPalette(p);
}

QPoint TilesetView::viewToTile(const QPoint &viewPos) const
{
    const QPoint scrollPos(horizontalScrollBar()->value(),
                          verticalScrollBar()->value());
    return (viewPos + scrollPos) / scale();
}

QRect TilesetView::viewToTile(const QRect &viewRect) const
{
    const QPoint scrollPos(horizontalScrollBar()->value(),
                          verticalScrollBar()->value());
    return QRect((viewRect.topLeft() + scrollPos) / scale(),
                 viewRect.size() / scale());
}

QRect TilesetView::tileToView(const QRect &tileRect) const
{
    const QPoint scrollPos(horizontalScrollBar()->value(),
                          verticalScrollBar()->value());
    return QRect((tileRect.topLeft() * scale()) - scrollPos,
                 tileRect.size() * scale());
}

void TilesetView::updateAtlasSelection(const QPoint &currentPos)
{
    QRect selection = QRect(mSelectionOffset, currentPos).normalized();

    if (mSnapToGrid) {
        const TilesetModel *model = tilesetModel();
        const int margin = model->tileset()->margin();
        const int spacing = model->tileset()->tileSpacing();
        const int tileWidth = model->tileset()->tileWidth();
        const int tileHeight = model->tileset()->tileHeight();

        // Calculate position relative to first tile (after margin)
        const int relX = qMax(selection.left() - margin, 0);
        const int relY = qMax(selection.top() - margin, 0);
        const int relRight = qMax(selection.right() - margin, 0);
        const int relBottom = qMax(selection.bottom() - margin, 0);

        // Find closest tile positions
        const int tileX = relX / (tileWidth + spacing);
        const int tileY = relY / (tileHeight + spacing);
        const int tileRight = (relRight + spacing) / (tileWidth + spacing);
        const int tileBottom = (relBottom + spacing) / (tileHeight + spacing);

        // Convert back to absolute coordinates with margin
        selection.setLeft(margin + tileX * (tileWidth + spacing));
        selection.setTop(margin + tileY * (tileHeight + spacing));
        selection.setRight(margin + tileRight * (tileWidth + spacing) + tileWidth - 1);
        selection.setBottom(margin + tileBottom * (tileHeight + spacing) + tileHeight - 1);
    }

    mRubberBand.setGeometry(tileToView(selection));
    mRubberBand.show();
}

#include "moc_tilesetview.cpp"
