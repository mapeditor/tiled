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

#include "changetileterrain.h"
#include "map.h"
#include "preferences.h"
#include "stylehelper.h"
#include "terrain.h"
#include "tile.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetmodel.h"
#include "utils.h"
#include "zoomable.h"

#include <QAbstractItemDelegate>
#include <QApplication>
#include <QCoreApplication>
#include <QGesture>
#include <QGestureEvent>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QPinchGesture>
#include <QScrollBar>
#include <QUndoCommand>
#include <QWheelEvent>
#include <QtCore/qmath.h>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

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

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    TilesetView *mTilesetView;
};

enum Corners
{
    TopLeft = 1,
    TopRight = 2,
    BottomLeft = 4,
    BottomRight = 8
};

/**
 * Returns a mask of the corners of a certain tile's \a terrain that contain
 * the given \a terrainTypeId.
 */
static unsigned terrainCorners(unsigned terrain, int terrainTypeId)
{
    const unsigned terrainIndex = terrainTypeId >= 0 ? terrainTypeId : 0xFF;

    return (((terrain >> 24) & 0xFF) == terrainIndex ? TopLeft : 0) |
            (((terrain >> 16) & 0xFF) == terrainIndex ? TopRight : 0) |
            (((terrain >> 8) & 0xFF) == terrainIndex ? BottomLeft : 0) |
            ((terrain & 0xFF) == terrainIndex ? BottomRight : 0);
}

static unsigned invertCorners(unsigned corners)
{
    return corners ^ (TopLeft | TopRight | BottomLeft | BottomRight);
}

static void paintCorners(QPainter *painter,
                         unsigned corners,
                         const QRect &rect)
{
    const int hx = rect.width() / 2;
    const int hy = rect.height() / 2;

    switch (corners) {
    case TopLeft:
        painter->drawPie(rect.translated(-hx, -hy), -90 * 16, 90 * 16);
        break;
    case TopRight:
        painter->drawPie(rect.translated(hx, -hy), 180 * 16, 90 * 16);
        break;
    case TopRight | TopLeft:
        painter->drawRect(rect.x(), rect.y(), rect.width(), hy);
        break;
    case BottomLeft:
        painter->drawPie(rect.translated(-hx, hy), 0, 90 * 16);
        break;
    case BottomLeft | TopLeft:
        painter->drawRect(rect.x(), rect.y(), hx, rect.height());
        break;
    case BottomLeft | TopRight:
        painter->drawPie(rect.translated(-hx, hy), 0, 90 * 16);
        painter->drawPie(rect.translated(hx, -hy), 180 * 16, 90 * 16);
        break;
    case BottomLeft | TopRight | TopLeft: {
        QPainterPath fill, ellipse;
        fill.addRect(rect);
        ellipse.addEllipse(rect.translated(hx, hy));
        painter->drawPath(fill.subtracted(ellipse));
        break;
    }
    case BottomRight:
        painter->drawPie(rect.translated(hx, hy), 90 * 16, 90 * 16);
        break;
    case BottomRight | TopLeft:
        painter->drawPie(rect.translated(-hx, -hy), -90 * 16, 90 * 16);
        painter->drawPie(rect.translated(hx, hy), 90 * 16, 90 * 16);
        break;
    case BottomRight | TopRight:
        painter->drawRect(rect.x() + hx, rect.y(), hx, rect.height());
        break;
    case BottomRight | TopRight | TopLeft: {
        QPainterPath fill, ellipse;
        fill.addRect(rect);
        ellipse.addEllipse(rect.translated(-hx, hy));
        painter->drawPath(fill.subtracted(ellipse));
        break;
    }
    case BottomRight | BottomLeft:
        painter->drawRect(rect.x(), rect.y() + hy, rect.width(), hy);
        break;
    case BottomRight | BottomLeft | TopLeft: {
        QPainterPath fill, ellipse;
        fill.addRect(rect);
        ellipse.addEllipse(rect.translated(hx, -hy));
        painter->drawPath(fill.subtracted(ellipse));
        break;
    }
    case BottomRight | BottomLeft | TopRight: {
        QPainterPath fill, ellipse;
        fill.addRect(rect);
        ellipse.addEllipse(rect.translated(-hx, -hy));
        painter->drawPath(fill.subtracted(ellipse));
        break;
    }
    case BottomRight | BottomLeft | TopRight | TopLeft:
        painter->drawRect(rect);
        break;
    }
}

static void setCosmeticPen(QPainter *painter, const QBrush &brush, qreal width)
{
#if QT_VERSION >= 0x050600
    QPen pen(brush, width * painter->device()->devicePixelRatioF());
#else
    QPen pen(brush, width * painter->device()->devicePixelRatio());
#endif
    pen.setCosmetic(true);
    painter->setPen(pen);
}

static void paintTerrainOverlay(QPainter *painter,
                                unsigned terrain,
                                int terrainTypeId,
                                const QRect &rect,
                                const QColor &color)
{
    painter->save();
    painter->setClipRect(rect);
    painter->setRenderHint(QPainter::Antialiasing);

    // Draw the "any terrain" background
    painter->setBrush(QColor(128, 128, 128, 100));
    setCosmeticPen(painter, Qt::gray, 2);
    paintCorners(painter, invertCorners(terrainCorners(terrain, -1)), rect);

    if (terrainTypeId != -1) {
        const unsigned corners = terrainCorners(terrain, terrainTypeId);

        // Draw the shadow
        painter->translate(1, 1);
        painter->setBrush(Qt::NoBrush);
        setCosmeticPen(painter, Qt::black, 2);
        paintCorners(painter, corners, rect);

        // Draw the foreground
        painter->translate(-1, -1);
        painter->setBrush(QColor(color.red(), color.green(), color.blue(), 100));
        setCosmeticPen(painter, color, 2);
        paintCorners(painter, corners, rect);
    }

    painter->restore();
}

static QTransform tilesetGridTransform(const Tileset &tileset, QPoint tileCenter)
{
    QTransform transform;

    if (tileset.orientation() == Tileset::Isometric) {
        const QSize gridSize = tileset.gridSize();

        transform.translate(tileCenter.x(), tileCenter.y());

        const auto ratio = (double) gridSize.height() / gridSize.width();
        const auto scaleX = 1.0 / sqrt(2.0);
        const auto scaleY = scaleX * ratio;
        transform.scale(scaleX, scaleY);

        transform.rotate(45.0);

        transform.translate(-tileCenter.x(), -tileCenter.y());
    }

    return transform;
}

void TileDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    const TilesetModel *model = static_cast<const TilesetModel*>(index.model());
    const Tile *tile = model->tileAt(index);
    if (!tile)
        return;

    const QPixmap &tileImage = tile->image();
    const int extra = mTilesetView->drawGrid() ? 1 : 0;
    const qreal zoom = mTilesetView->scale();

    QSize tileSize = tileImage.size();
    if (tileImage.isNull()) {
        Tileset *tileset = model->tileset();
        if (tileset->isCollection()) {
            tileSize = QSize(32, 32);
        } else {
            int max = std::max(tileset->tileWidth(), tileset->tileWidth());
            int min = std::min(max, 32);
            tileSize = QSize(min, min);
        }
    }
    tileSize *= zoom;

    // Compute rectangle to draw the image in: bottom- and left-aligned
    QRect targetRect = option.rect.adjusted(0, 0, -extra, -extra);
    targetRect.setTop(targetRect.bottom() - tileSize.height() + 1);
    targetRect.setRight(targetRect.left() + tileSize.width() - 1);

    // Draw the tile image
    if (Zoomable *zoomable = mTilesetView->zoomable())
        if (zoomable->smoothTransform())
            painter->setRenderHint(QPainter::SmoothPixmapTransform);

    if (!tileImage.isNull())
        painter->drawPixmap(targetRect, tileImage);
    else
        mTilesetView->imageMissingIcon().paint(painter, targetRect, Qt::AlignBottom | Qt::AlignLeft);


    // Overlay with film strip when animated
    if (mTilesetView->markAnimatedTiles() && tile->isAnimated()) {
        painter->save();

        qreal scale = qMin(tileImage.width() / 32.0,
                           tileImage.height() / 32.0);

        painter->setClipRect(targetRect);
        painter->translate(targetRect.right(),
                           targetRect.bottom());
        painter->scale(scale * zoom, scale * zoom);
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

    const auto highlight = option.palette.highlight();

    // Overlay with highlight color when selected
    if (option.state & QStyle::State_Selected) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.5);
        painter->fillRect(targetRect, highlight);
        painter->setOpacity(opacity);
    }

    if (mTilesetView->isEditTerrain()) {
        painter->save();
        painter->setTransform(tilesetGridTransform(*tile->tileset(), targetRect.center()), true);

        const unsigned terrain = tile->terrain();

        paintTerrainOverlay(painter, terrain,
                            mTilesetView->terrainId(), targetRect,
                            highlight.color());

        // Overlay with terrain corner indication when hovered
        if (index == mTilesetView->hoveredIndex()) {
            QPoint pos;
            switch (mTilesetView->hoveredCorner()) {
            case 0: pos = targetRect.topLeft(); break;
            case 1: pos = targetRect.topRight(); break;
            case 2: pos = targetRect.bottomLeft(); break;
            case 3: pos = targetRect.bottomRight(); break;
            }

            painter->save();
            painter->setBrush(option.palette.highlight());
            painter->setClipRect(targetRect);
            painter->setRenderHint(QPainter::Antialiasing);
            setCosmeticPen(painter, highlight.color().darker(), 2);
            painter->drawEllipse(pos,
                                 targetRect.width() / 3,
                                 targetRect.height() / 3);
            painter->restore();
        }

        painter->restore();
    }
}

QSize TileDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                             const QModelIndex &index) const
{
    const TilesetModel *m = static_cast<const TilesetModel*>(index.model());
    const int extra = mTilesetView->drawGrid() ? 1 : 0;

    if (const Tile *tile = m->tileAt(index)) {
        const QPixmap &image = tile->image();
        QSize tileSize = image.size();

        if (image.isNull()) {
            Tileset *tileset = m->tileset();
            if (tileset->isCollection()) {
                tileSize = QSize(32, 32);
            } else {
                int max = std::max(tileset->tileWidth(), tileset->tileWidth());
                int min = std::min(max, 32);
                tileSize = QSize(min, min);
            }
        }

        return QSize(tileSize.width() * mTilesetView->scale() + extra,
                     tileSize.height() * mTilesetView->scale() + extra);
    }

    return QSize(extra, extra);
}

} // anonymous namespace


TilesetView::TilesetView(QWidget *parent)
    : QTableView(parent)
    , mZoomable(nullptr)
    , mTilesetDocument(nullptr)
    , mMarkAnimatedTiles(true)
    , mEditTerrain(false)
    , mEraseTerrain(false)
    , mTerrain(nullptr)
    , mHoveredCorner(0)
    , mTerrainChanged(false)
    , mHandScrolling(false)
    , mImageMissingIcon(QStringLiteral("://images/32x32/image-missing.png"))
{
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setItemDelegate(new TileDelegate(this, this));
    setShowGrid(false);

    QHeaderView *hHeader = horizontalHeader();
    QHeaderView *vHeader = verticalHeader();
    hHeader->hide();
    vHeader->hide();
    hHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    vHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    hHeader->setMinimumSectionSize(1);
    vHeader->setMinimumSectionSize(1);

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
}

void TilesetView::setTilesetDocument(TilesetDocument *tilesetDocument)
{
    mTilesetDocument = tilesetDocument;
}

QSize TilesetView::sizeHint() const
{
    return QSize(130, 100);
}

int TilesetView::sizeHintForColumn(int column) const
{
    Q_UNUSED(column)
    const TilesetModel *model = tilesetModel();
    if (!model)
        return -1;
    if (model->tileset()->isCollection())
        return QTableView::sizeHintForColumn(column);

    const int tileWidth = model->tileset()->tileWidth();
    return qRound(tileWidth * scale()) + (mDrawGrid ? 1 : 0);
}

int TilesetView::sizeHintForRow(int row) const
{
    Q_UNUSED(row)
    const TilesetModel *model = tilesetModel();
    if (!model)
        return -1;
    if (model->tileset()->isCollection())
        return QTableView::sizeHintForRow(row);

    const int tileHeight = model->tileset()->tileHeight();
    return qRound(tileHeight * scale()) + (mDrawGrid ? 1 : 0);
}

void TilesetView::setZoomable(Zoomable *zoomable)
{
    if (mZoomable)
        mZoomable->disconnect(this);

    if (zoomable)
        connect(zoomable, SIGNAL(scaleChanged(qreal)), SLOT(adjustScale()));

    mZoomable = zoomable;
    adjustScale();
}

qreal TilesetView::scale() const
{
    return mZoomable ? mZoomable->scale() : 1;
}

void TilesetView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    updateBackgroundColor();
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
    if (mZoomable && event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
        if (QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture))
            mZoomable->handlePinchGesture(static_cast<QPinchGesture *>(gesture));
    }

    return QTableView::event(event);
}

void TilesetView::setEditTerrain(bool enabled)
{
    if (mEditTerrain == enabled)
        return;

    mEditTerrain = enabled;
    setMouseTracking(true);
    viewport()->update();
}

/**
 * The id of the terrain currently being specified. Returns -1 when no terrain
 * is set (used for erasing terrain info).
 */
int TilesetView::terrainId() const
{
     return mTerrain ? mTerrain->id() : -1;
}

void TilesetView::setTerrain(const Terrain *terrain)
{
    if (mTerrain == terrain)
        return;

    mTerrain = terrain;
    if (mEditTerrain)
        viewport()->update();
}

QIcon TilesetView::imageMissingIcon() const
{
    return QIcon::fromTheme(QLatin1String("image-missing"), mImageMissingIcon);
}

void TilesetView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton && isActiveWindow()) {
        mLastMousePos = event->globalPos();
        setHandScrolling(true);
        return;
    }

    if (!mEditTerrain) {
        QTableView::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton)
        applyTerrain();
}

void TilesetView::mouseMoveEvent(QMouseEvent *event)
{
    if (mHandScrolling) {
        auto *hBar = horizontalScrollBar();
        auto *vBar = verticalScrollBar();
        const QPoint d = event->globalPos() - mLastMousePos;

        int horizontalValue = hBar->value() + (isRightToLeft() ? d.x() : -d.x());
        int verticalValue = vBar->value() - d.y();

        hBar->setValue(horizontalValue);
        vBar->setValue(verticalValue);

        mLastMousePos = event->globalPos();
        return;
    }

    if (!mEditTerrain) {
        QTableView::mouseMoveEvent(event);
        return;
    }

    const QPoint pos = event->pos();
    const QModelIndex hoveredIndex = indexAt(pos);
    int hoveredCorner = 0;

    if (hoveredIndex.isValid()) {
        const QPoint center = visualRect(hoveredIndex).center();

        const auto t = tilesetGridTransform(*tilesetDocument()->tileset(), center);
        const auto mappedPos = t.inverted().map(pos);

        if (mappedPos.x() > center.x())
            hoveredCorner += 1;
        if (mappedPos.y() > center.y())
            hoveredCorner += 2;
    }

    if (mHoveredIndex != hoveredIndex || mHoveredCorner != hoveredCorner) {
        const QModelIndex previousHoveredIndex = mHoveredIndex;
        mHoveredIndex = hoveredIndex;
        mHoveredCorner = hoveredCorner;

        if (previousHoveredIndex.isValid())
            update(previousHoveredIndex);
        if (previousHoveredIndex != mHoveredIndex && mHoveredIndex.isValid())
            update(mHoveredIndex);
    }

    if (event->buttons() & Qt::LeftButton)
        applyTerrain();
}

void TilesetView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        setHandScrolling(false);
        return;
    }

    if (!mEditTerrain) {
        QTableView::mouseReleaseEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton)
        finishTerrainChange();
}

void TilesetView::leaveEvent(QEvent *event)
{
    if (mHoveredIndex.isValid()) {
        const QModelIndex previousHoveredIndex = mHoveredIndex;
        mHoveredIndex = QModelIndex();
        update(previousHoveredIndex);
    }

    QTableView::leaveEvent(event);
}

/**
 * Override to support zooming in and out using the mouse wheel.
 */
void TilesetView::wheelEvent(QWheelEvent *event)
{
    if (mZoomable &&
            event->modifiers() & Qt::ControlModifier &&
            event->orientation() == Qt::Vertical)
    {
        mZoomable->handleWheelDelta(event->delta());
        return;
    }

    QTableView::wheelEvent(event);
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

    Tile *tile = model->tileAt(index);

    QMenu menu;

    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));

    if (tile) {
        if (mEditTerrain) {
            // Select this tile to make sure it is clear that only a single
            // tile is being used.
            selectionModel()->setCurrentIndex(index,
                                              QItemSelectionModel::SelectCurrent |
                                              QItemSelectionModel::Clear);

            QAction *addTerrain = menu.addAction(tr("Add Terrain Type"));
            connect(addTerrain, SIGNAL(triggered()), SLOT(addTerrainType()));

            if (mTerrain) {
                QAction *setImage = menu.addAction(tr("Set Terrain Image"));
                connect(setImage, SIGNAL(triggered()), SLOT(selectTerrainImage()));
            }
        } else if (mTilesetDocument) {
            QAction *tileProperties = menu.addAction(propIcon,
                                                     tr("Tile &Properties..."));
            Utils::setThemeIcon(tileProperties, "document-properties");
            connect(tileProperties, SIGNAL(triggered()),
                    SLOT(editTileProperties()));
        }

        menu.addSeparator();
    }

    QAction *toggleGrid = menu.addAction(tr("Show &Grid"));
    toggleGrid->setCheckable(true);
    toggleGrid->setChecked(mDrawGrid);

    Preferences *prefs = Preferences::instance();
    connect(toggleGrid, SIGNAL(toggled(bool)),
            prefs, SLOT(setShowTilesetGrid(bool)));

    menu.exec(event->globalPos());
}

void TilesetView::addTerrainType()
{
    if (Tile *tile = currentTile())
        emit createNewTerrain(tile);
}

void TilesetView::selectTerrainImage()
{
    if (Tile *tile = currentTile())
        emit terrainImageSelected(tile);
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

void TilesetView::setDrawGrid(bool drawGrid)
{
    mDrawGrid = drawGrid;
    if (TilesetModel *model = tilesetModel())
        model->resetModel();
}

void TilesetView::adjustScale()
{
    if (TilesetModel *model = tilesetModel())
        model->resetModel();
}

void TilesetView::applyTerrain()
{
    if (!mHoveredIndex.isValid())
        return;

    Tile *tile = tilesetModel()->tileAt(mHoveredIndex);
    if (!tile)
        return;

    unsigned terrain = setTerrainCorner(tile->terrain(),
                                        mHoveredCorner,
                                        mEraseTerrain ? 0xFF : terrainId());

    if (terrain == tile->terrain())
        return;

    QUndoCommand *command = new ChangeTileTerrain(mTilesetDocument, tile, terrain);
    mTilesetDocument->undoStack()->push(command);
    mTerrainChanged = true;
}

void TilesetView::finishTerrainChange()
{
    if (!mTerrainChanged)
        return;

    // Prevent further merging since mouse was released
    mTilesetDocument->undoStack()->push(new ChangeTileTerrain);
    mTerrainChanged = false;
}

Tile *TilesetView::currentTile() const
{
    const TilesetModel *model = tilesetModel();
    return model ? model->tileAt(currentIndex()) : nullptr;
}

void TilesetView::setHandScrolling(bool handScrolling)
{
    if (mHandScrolling == handScrolling)
        return;

    mHandScrolling = handScrolling;

    if (mHandScrolling)
        setCursor(QCursor(Qt::ClosedHandCursor));
    else
        unsetCursor();
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
