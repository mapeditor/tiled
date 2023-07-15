/*
 * maprenderer.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "maprenderer.h"

#include "imagelayer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "staggeredrenderer.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCache>
#include <QPaintEngine>
#include <QPainter>
#include <QVector2D>

#include <cmath>

using namespace Tiled;

struct TintedKey
{
    const qint64 key;
    const QColor color;

    bool operator==(const TintedKey &o) const
    {
        return key == o.key && color == o.color;
    }
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
uint qHash(const TintedKey &key, uint seed) Q_DECL_NOTHROW
#else
size_t qHash(const TintedKey &key, size_t seed) Q_DECL_NOTHROW
#endif
{
    auto h = ::qHash(key.key, seed);
    h = ::qHash(key.color.rgba(), h);
    return h;
}

// Borrowed from qpixmapcache.cpp
static inline qsizetype cost(const QPixmap &pixmap)
{
    // make sure to do a 64bit calculation; qsizetype might be smaller
    const qint64 costKb = static_cast<qint64>(pixmap.width())
                        * pixmap.height() * pixmap.depth() / (8 * 1024);
    const qint64 costMax = std::numeric_limits<qsizetype>::max();
    // a small pixmap should have at least a cost of 1(kb)
    return static_cast<qsizetype>(qBound(1LL, costKb, costMax));
}

static QPixmap tinted(const QPixmap &pixmap, const QColor &color)
{
    if (!color.isValid() || color == QColor(255, 255, 255, 255) || pixmap.isNull())
        return pixmap;

    // Cache for up to 100 MB of tinted pixmaps, since tinting is expensive
    static QCache<TintedKey, QPixmap> cache { 100 * 1024 };

    const TintedKey tintedKey { pixmap.cacheKey(), color };
    if (auto cached = cache.object(tintedKey))
        return *cached;

    QPixmap resultImage = pixmap;
    QPainter painter(&resultImage);

    QColor fullOpacity = color;
    fullOpacity.setAlpha(255);
    // tint the final color (this will will mess up the alpha which we will fix
    // in the next lines)
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(resultImage.rect(), fullOpacity);

    // apply the original alpha to the final image
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, pixmap);

    // apply the alpha of the tint color so that we can use it to make the image
    // transparent instead of just increasing or decreasing the tint effect
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.fillRect(resultImage.rect(), color);

    painter.end();

    cache.insert(tintedKey, new QPixmap(resultImage), cost(resultImage));

    return resultImage;
}

MapRenderer::~MapRenderer()
{}

QRect MapRenderer::mapBoundingRect() const
{
    return boundingRect(map()->tileBoundingRect());
}

QRectF MapRenderer::boundingRect(const ImageLayer *imageLayer) const
{
    QRectF bounds = QRectF(QPointF(), imageLayer->image().size());

    if (imageLayer->repeatX()) {
        bounds.setLeft(INT_MIN / 512);
        bounds.setRight(INT_MAX / 256);
    }
    if (imageLayer->repeatY()) {
        bounds.setTop(INT_MIN / 512);
        bounds.setBottom(INT_MAX / 256);
    }

    return bounds;
}

QPainterPath MapRenderer::pointShape(const QPointF &position) const
{
    QPainterPath path;

    const qreal radius = 10.0;
    const qreal sweep = 235.0;
    const qreal startAngle = 90.0 - sweep / 2;
    QRectF rectangle(-radius, -radius, radius * 2, radius * 2);
    path.moveTo(radius * cos(startAngle * M_PI / 180.0), -radius * sin(startAngle * M_PI / 180.0));
    path.arcTo(rectangle, startAngle, sweep);
    path.lineTo(0, 2 * radius);
    path.closeSubpath();

    QPainterPath hole;
    const qreal smallRadius = radius / 2.0;
    hole.addEllipse(QRectF(-smallRadius, -smallRadius, smallRadius * 2, smallRadius * 2));
    path = path.subtracted(hole);

    path.translate(pixelToScreenCoords(position) +
                   QPointF(0, -2 * radius));

    return path;
}

void MapRenderer::drawImageLayer(QPainter *painter,
                                 const ImageLayer *imageLayer,
                                 const QRectF &exposed) const
{
    painter->save();
    painter->setBrush(tinted(imageLayer->image(), imageLayer->effectiveTintColor()));
    painter->setPen(Qt::NoPen);
    if (exposed.isNull())
        painter->drawRect(boundingRect(imageLayer));
    else
        painter->drawRect(boundingRect(imageLayer) & exposed);
    painter->restore();
}

void MapRenderer::drawPointObject(QPainter *painter, const QColor &color) const
{
    const qreal lineWidth = objectLineWidth();
    const qreal scale = painterScale();
    const qreal shadowDist = (lineWidth == 0 ? 1 : lineWidth) / scale;
    const QPointF shadowOffset = QPointF(shadowDist * 0.5,
                                         shadowDist * 0.5);

    QPen linePen(color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    linePen.setCosmetic(true);
    QPen shadowPen(linePen);
    shadowPen.setColor(Qt::black);

    QColor brushColor = color;
    brushColor.setAlpha(50);
    const QBrush fillBrush(brushColor);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fillBrush);

    QPainterPath path;

    const qreal radius = 10.0;
    const qreal sweep = 235.0;
    const qreal startAngle = 90.0 - sweep / 2;
    QRectF rectangle(-radius, -radius, radius * 2, radius * 2);
    path.moveTo(radius * cos(startAngle * M_PI / 180.0), -radius * sin(startAngle * M_PI / 180.0));
    path.arcTo(rectangle, startAngle, sweep);
    path.lineTo(0, 2 * radius);
    path.closeSubpath();

    painter->translate(0, -2 * radius);

    painter->setPen(shadowPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path.translated(shadowOffset));

    painter->setPen(linePen);
    painter->setBrush(fillBrush);
    painter->drawPath(path);

    const QBrush opaqueBrush(color);
    painter->setBrush(opaqueBrush);
    const qreal smallRadius = radius / 3.0;
    painter->drawEllipse(QRectF(-smallRadius, -smallRadius, smallRadius * 2, smallRadius * 2));
}

QPainterPath MapRenderer::pointInteractionShape(const MapObject *object) const
{
    Q_ASSERT(object->shape() == MapObject::Point);
    QPainterPath path;
    path.addRect(QRect(-10, -30, 20, 30));
    path.translate(pixelToScreenCoords(object->position()));
    return path;
}

QPointF MapRenderer::snapToGrid(const QPointF &pixelCoords, int subdivisions) const
{
    QPointF tileCoords = pixelToTileCoords(pixelCoords);
    if (subdivisions > 1) {
        tileCoords = (tileCoords * subdivisions).toPoint();
        tileCoords /= subdivisions;
    } else {
        tileCoords = tileCoords.toPoint();
    }
    return tileToPixelCoords(tileCoords);
}

void MapRenderer::drawTileLayer(QPainter *painter, const TileLayer *layer, const QRectF &exposed) const
{
    const QSize tileSize = map()->tileSize();

    // Don't draw more than the bounding rectangle of the given layer,
    // intersected with the exposed rectangle.
    QRect rect = boundingRect(layer->bounds());
    if (!exposed.isNull())
        rect &= exposed.toAlignedRect();

    // Draw margins extend the rendered area on the opposite side. We subtract
    // the grid size because this has already been taken into account by
    // boundingRect.
    QMargins drawMargins = layer->drawMargins();
    drawMargins.setTop(qMax(0, drawMargins.top() - tileSize.height()));
    drawMargins.setRight(qMax(0, drawMargins.right() - tileSize.width()));
    rect.adjust(-drawMargins.right(),
                -drawMargins.bottom(),
                drawMargins.left(),
                drawMargins.top());

    CellRenderer renderer(painter, this, layer->effectiveTintColor());

    auto tileRenderFunction = [layer, &renderer, tileSize](QPoint tilePos, const QPointF &screenPos) {
        const Cell &cell = layer->cellAt(tilePos - layer->position());
        if (!cell.isEmpty()) {
            QSize size = tileSize;

            if (cell.tileset()->tileRenderSize() == Tileset::TileSize) {
                if (const Tile *tile = cell.tile())
                    size = tile->size();
            }

            renderer.render(cell, screenPos, size, CellRenderer::BottomLeft);
        }
    };

    drawTileLayer(tileRenderFunction, rect);
}

void MapRenderer::setFlag(RenderFlag flag, bool enabled)
{
    mFlags.setFlag(flag, enabled);
}

/**
 * Converts a line running from \a start to \a end to a polygon which
 * extends 5 pixels from the line in all directions.
 */
QPolygonF MapRenderer::lineToPolygon(const QPointF &start, const QPointF &end)
{
    QPointF direction = QVector2D(end - start).normalized().toPointF();
    QPointF perpendicular(-direction.y(), direction.x());

    const qreal thickness = 5.0; // 5 pixels on each side
    direction *= thickness;
    perpendicular *= thickness;

    QPolygonF polygon(4);
    polygon[0] = start + perpendicular - direction;
    polygon[1] = start - perpendicular - direction;
    polygon[2] = end - perpendicular + direction;
    polygon[3] = end + perpendicular + direction;
    return polygon;
}

/**
 * Returns a MapRenderer instance matching the orientation of the map.
 */
std::unique_ptr<MapRenderer> MapRenderer::create(const Map *map)
{
    switch (map->orientation()) {
    case Map::Isometric:
        return std::make_unique<IsometricRenderer>(map);
    case Map::Staggered:
        return std::make_unique<StaggeredRenderer>(map);
    case Map::Hexagonal:
        return std::make_unique<HexagonalRenderer>(map);
    default:
        return std::make_unique<OrthogonalRenderer>(map);
    }
}

void MapRenderer::setupGridPens(const QPaintDevice *device, QColor color,
                                QPen &gridPen, QPen &majorGridPen, int gridSize,
                                QSize gridMajor) const
{
    const qreal devicePixelRatio = device->devicePixelRatioF();

#ifdef Q_OS_MAC
    const qreal dpiScale = 1.0;
#else
    const qreal dpiScale = device->logicalDpiX() / 96.0;
#endif

    const auto majorGridSize = qMax(gridMajor.width(), gridMajor.height()) * gridSize;
    const qreal dashLength = std::ceil(2.0 * dpiScale);
    const qreal autoAlpha = qBound(0.0, (gridSize * mPainterScale - 3) / 17.0, 1.0);
    const qreal majorAutoAlpha = qBound(0.0, (majorGridSize * mPainterScale - 3) / 17.0, 1.0);

    color.setAlpha(96 * autoAlpha);

    gridPen = QPen(color, 1.0 * devicePixelRatio);
    gridPen.setCosmetic(true);
    gridPen.setDashPattern({dashLength, dashLength});

    color.setAlpha(96 * autoAlpha + 96 * majorAutoAlpha);

    majorGridPen = gridPen;
    majorGridPen.setColor(color);
}


static void renderMissingImageMarker(QPainter &painter, const QRectF &rect)
{
    QRectF r { rect.adjusted(0.5, 0.5, -0.5, -0.5) };
    QPen pen { Qt::red, 1 };
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    painter.save();
    painter.fillRect(r, QColor(0, 0, 0, 128));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.drawRect(r);
    painter.drawLine(r.topLeft(), r.bottomRight());
    painter.drawLine(r.topRight(), r.bottomLeft());
    painter.restore();
}

static bool hasOpenGLEngine(const QPainter *painter)
{
    if (auto paintEngine = painter->paintEngine()) {
        const QPaintEngine::Type type = paintEngine->type();
        return (type == QPaintEngine::OpenGL ||
                type == QPaintEngine::OpenGL2);
    }
    return false;
}

CellRenderer::CellRenderer(QPainter *painter, const MapRenderer *renderer, const QColor &tintColor)
    : mPainter(painter)
    , mRenderer(renderer)
    , mTile(nullptr)
    , mIsOpenGL(hasOpenGLEngine(painter))
    , mTintColor(tintColor)
{
}

/**
 * Renders a \a cell with the given \a origin at \a pos, taking into account
 * the flipping and tile offset.
 *
 * For performance reasons, the actual drawing is delayed until a different
 * kind of tile has to be drawn. For this reason it is necessary to call
 * flush when finished doing drawCell calls. This function is also called by
 * the destructor so usually an explicit call is not needed.
 *
 * This call expects `painter.translate(pos)` to correspond to the Origin point.
 */
void CellRenderer::render(const Cell &cell, const QPointF &screenPos, const QSizeF &size, Origin origin)
{
    const Tile *tile = cell.tile();

    if (tile && mRenderer->testFlag(ShowTileAnimations))
        tile = tile->currentFrameTile();

    if (!tile || tile->image().isNull()) {
        QRectF target { screenPos, size };

        if (origin == BottomLeft)
            target.translate(0.0, -size.height());

        renderMissingImageMarker(*mPainter, target);
        return;
    }

    // The USHRT_MAX limit is rather arbitrary but avoids a crash in
    // drawPixmapFragments for a large number of fragments.
    if (mTile != tile || mFragments.size() == USHRT_MAX)
        flush();

    const QPixmap &image = tile->image();
    const QRect imageRect = tile->imageRect();
    if (imageRect.isEmpty())
        return;

    const QPoint offset = tile->offset();
    const QPointF sizeHalf { size.width() / 2, size.height() / 2 };

    bool flippedHorizontally = cell.flippedHorizontally();
    bool flippedVertically = cell.flippedVertically();

    QPainter::PixmapFragment fragment;
    // Calculate the position as if the origin is TopLeft, and correct it later.
    fragment.x = screenPos.x() + sizeHalf.x();
    fragment.y = screenPos.y() + sizeHalf.y();
    fragment.sourceLeft = imageRect.x();
    fragment.sourceTop = imageRect.y();
    fragment.width = imageRect.width();
    fragment.height = imageRect.height();
    fragment.scaleX = size.width() / imageRect.width();
    fragment.scaleY = size.height() / imageRect.height();
    fragment.rotation = 0;
    fragment.opacity = 1;

    const auto fillMode = tile->tileset()->fillMode();
    if (fillMode == Tileset::PreserveAspectFit) {
        const auto minScale = std::min(fragment.scaleX, fragment.scaleY);
        fragment.scaleX = minScale;
        fragment.scaleY = minScale;
    }

    fragment.x += offset.x() * fragment.scaleX;
    fragment.y += offset.y() * fragment.scaleY;

    // Correct the position if the origin is BottomLeft.
    if (origin == BottomLeft)
        fragment.y -= size.height();

    if (mRenderer->cellType() == MapRenderer::HexagonalCells) {

        if (cell.flippedAntiDiagonally())
            fragment.rotation += 60;

        if (cell.rotatedHexagonal120())
            fragment.rotation += 120;

    } else if (cell.flippedAntiDiagonally()) {
        fragment.rotation = 90;

        flippedHorizontally = flippedVertically;
        flippedVertically = !cell.flippedHorizontally();

        // Compensate for the swap of image dimensions
        const qreal halfDiff = sizeHalf.y() - sizeHalf.x();
        fragment.y += halfDiff;
        fragment.x += halfDiff;
    }

    fragment.scaleX *= flippedHorizontally ? -1 : 1;
    fragment.scaleY *= flippedVertically ? -1 : 1;

    // Avoid using drawPixmapFragments with OpenGL in Qt 6.4.1 and above
    // (https://bugreports.qt.io/browse/QTBUG-111416)
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 1) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (mIsOpenGL || (fragment.scaleX > 0 && fragment.scaleY > 0)) {
#else
    if (!mIsOpenGL && fragment.scaleX > 0 && fragment.scaleY > 0) {
#endif
        mTile = tile;
        mFragments.append(fragment);
        return;
    }

    // The Raster paint engine as of Qt 4.8.4 / 5.0.2 does not support
    // drawing fragments with a negative scaling factor.

    flush(); // make sure we drew all tiles so far

    const QTransform oldTransform = mPainter->transform();
    QTransform transform = oldTransform;
    transform.translate(fragment.x, fragment.y);
    transform.rotate(fragment.rotation);
    transform.scale(fragment.scaleX, fragment.scaleY);

    const QRectF target(fragment.width * -0.5, fragment.height * -0.5,
                        fragment.width, fragment.height);
    const QRectF source(fragment.sourceLeft, fragment.sourceTop,
                        fragment.width, fragment.height);

    mPainter->setTransform(transform);
    mPainter->drawPixmap(target, tinted(image, mTintColor), source);
    mPainter->setTransform(oldTransform);

    // A bit of a hack to still draw tile collision shapes when requested
    if (mRenderer->flags().testFlag(ShowTileCollisionShapes)
            && tile->objectGroup()
            && !tile->objectGroup()->objects().isEmpty()) {
        mTile = tile;
        mFragments.append(fragment);
        paintTileCollisionShapes();
        mTile = nullptr;
        mFragments.clear();
    }
}

/**
 * Renders any remaining cells.
 */
void CellRenderer::flush()
{
    if (!mTile)
        return;

    mPainter->drawPixmapFragments(mFragments.constData(),
                                  mFragments.size(),
                                  tinted(mTile->image(), mTintColor));

    if (mRenderer->flags().testFlag(ShowTileCollisionShapes)
            && mTile->objectGroup()
            && !mTile->objectGroup()->objects().isEmpty()) {
        paintTileCollisionShapes();
    }

    mTile = nullptr;
    mFragments.clear();
}

/**
 * Returns a transform that rotates by \a rotation degrees around the given
 * \a position.
 */
static QTransform rotateAt(const QPointF &position, qreal rotation)
{
    QTransform transform;
    transform.translate(position.x(), position.y());
    transform.rotate(rotation);
    transform.translate(-position.x(), -position.y());
    return transform;
}

void CellRenderer::paintTileCollisionShapes()
{
    const Tileset *tileset = mTile->tileset();
    const bool isIsometric = tileset->orientation() == Tileset::Isometric;
    Map::Parameters mapParameters;
    mapParameters.orientation = isIsometric ? Map::Isometric : Map::Orthogonal;
    mapParameters.width = 1;
    mapParameters.height = 1;
    mapParameters.tileWidth = tileset->gridSize().width();
    mapParameters.tileHeight = tileset->gridSize().height();
    const Map map(mapParameters);
    const auto renderer = MapRenderer::create(&map);

    const qreal lineWidth = mRenderer->objectLineWidth();
    const qreal shadowDist = (lineWidth == 0 ? 1 : lineWidth) / mRenderer->painterScale();
    const QPointF shadowOffset = QPointF(shadowDist * 0.5, shadowDist * 0.5);

    QPen shadowPen(Qt::black);
    shadowPen.setCosmetic(true);
    shadowPen.setJoinStyle(Qt::RoundJoin);
    shadowPen.setCapStyle(Qt::RoundCap);
    shadowPen.setWidthF(lineWidth);
    shadowPen.setStyle(Qt::DotLine);

    mPainter->setRenderHint(QPainter::Antialiasing);

    for (const auto &fragment : std::as_const(mFragments)) {
        QTransform tileTransform;
        tileTransform.translate(fragment.x, fragment.y);
        tileTransform.rotate(fragment.rotation);
        tileTransform.scale(fragment.scaleX, fragment.scaleY);
        tileTransform.translate(-fragment.width * 0.5, -fragment.height * 0.5);

        if (isIsometric)
            tileTransform.translate(0, fragment.height - tileset->gridSize().height());

        for (MapObject *object : mTile->objectGroup()->objects()) {
            QColor penColor = object->effectiveColor();
            QColor brushColor = penColor;
            brushColor.setAlpha(50);
            QPen colorPen(shadowPen);
            colorPen.setColor(penColor);

            mPainter->setPen(colorPen);
            mPainter->setBrush(brushColor);

            auto transform = rotateAt(renderer->pixelToScreenCoords(object->position()),
                                      object->rotation());
            transform *= tileTransform;

            const auto shape = transform.map(renderer->shape(object));

            mPainter->strokePath(shape.translated(shadowOffset), shadowPen);

            if (object->shape() == MapObject::Polyline)
                mPainter->strokePath(shape, colorPen);
            else
                mPainter->drawPath(shape);
        }
    }
}
