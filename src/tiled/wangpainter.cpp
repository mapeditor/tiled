#include "wangpainter.h"
#include "grid.h"
#include "geometry.h"
#include "wangfiller.h"
#include "hexagonalrenderer.h"

namespace Tiled {

static constexpr QPoint aroundTilePoints[WangId::NumIndexes] = {
    QPoint( 0, -1),
    QPoint( 1, -1),
    QPoint( 1,  0),
    QPoint( 1,  1),
    QPoint( 0,  1),
    QPoint(-1,  1),
    QPoint(-1,  0),
    QPoint(-1, -1)
};

//  3 0
//  2 1
static constexpr QPoint aroundVertexPoints[WangId::NumCorners] = {
    QPoint( 0, -1),
    QPoint( 0,  0),
    QPoint(-1,  0),
    QPoint(-1, -1)
};

WangPainter::WangPainter() {}

WangPainter::~WangPainter() {}

void WangPainter::setWangSet(const WangSet *wangSet) {
    if (wangSet == mWangSet) {
        return;
    }

    mCurrentColor = 0;
    mWangSet = wangSet;

    if (mWangSet)
    {
        switch (mWangSet->type())
        {
        case WangSet::Corner:
            mBrushMode = WangBrush::BrushMode::PaintCorner;
            break;
        case WangSet::Edge:
            mBrushMode = WangBrush::BrushMode::PaintEdge;
            break;
        case WangSet::Mixed:
        {
            mBrushMode = WangBrush::BrushMode::PaintEdgeAndCorner;
            break;
        }
        }
    }
    else
    {
        mBrushMode = WangBrush::BrushMode::Idle;
    }
}

void WangPainter::setColor(int color) {
    if (color == mCurrentColor) {
        return;
    }
    mCurrentColor = color;

    if (!mWangSet)
        return;

    switch (mWangSet->type())
    {
        case WangSet::Corner:
            mBrushMode = WangBrush::BrushMode::PaintCorner;
            break;
        case WangSet::Edge:
            mBrushMode = WangBrush::BrushMode::PaintEdge;
            break;
        case WangSet::Mixed:
        {
            // Determine a meaningful mode by looking at where the color is used.
            bool usedAsCorner = false;
            bool usedAsEdge = false;

            if (mWangSet && color > 0 && color <= mWangSet->colorCount())
            {
                for (const WangId wangId : mWangSet->wangIdByTileId())
                {
                    for (int i = 0; i < WangId::NumIndexes; ++i)
                    {
                        if (wangId.indexColor(i) == color)
                        {
                            const bool isCorner = WangId::isCorner(i);
                            usedAsCorner |= isCorner;
                            usedAsEdge |= !isCorner;
                        }
                    }
                }
            }

            if (usedAsEdge == usedAsCorner)
                mBrushMode = WangBrush::BrushMode::PaintEdgeAndCorner;
            else if (usedAsEdge)
                mBrushMode = WangBrush::BrushMode::PaintEdge;
            else
                mBrushMode = WangBrush::BrushMode::PaintCorner;

            break;
        }
    }
}

void WangPainter::setPosition(QPoint pos) {
    if (mBrushMode == WangBrush::BrushMode::Idle) {
        return;
    }
    mCurrentPosition = pos;
}

void WangPainter::setDirection(WangId::Index directionToGenerate) {
    if (mBrushMode == WangBrush::BrushMode::Idle) {
        return;
    }
    
    // TODO: for PaintCorner mode, this is irrelevant. For PaintEdge and PaintEdgeAndCorner, we actually
    // need to know what wang index to use when generating the stamp. 
    mWangIndex = directionToGenerate;

    switch (mBrushMode) {
        case WangBrush::BrushMode::Idle:              // can't happen due to check above
            return;
        case WangBrush::BrushMode::PaintCorner:
            // override this because it should always be topLeft for PaintCorner.
            mWangIndex = WangId::TopLeft;
            break;
        case WangBrush::BrushMode::PaintEdge: 
            // no corners, so we have to set these to cardinal coordinates.
            switch (mWangIndex) {
                case WangId::BottomRight:
                    
                    mWangIndex = WangId::Bottom;
                    break;
                case WangId::BottomLeft:
                    mWangIndex = WangId::Left;
                    break;
                case WangId::TopLeft:
                    mWangIndex = WangId::Top;
                    break;
                case WangId::TopRight:
                    mWangIndex = WangId::Right;
                    break;
                default:
                    break;
            }
            break;
        
        case WangBrush::BrushMode::PaintEdgeAndCorner:
            switch (mWangIndex) {
                case WangId::BottomRight:
                    mWangIndex = WangId::TopLeft;
                    break;
                case WangId::BottomLeft:
                    mWangIndex = WangId::TopLeft;
                    break;
                case WangId::TopRight:
                    mWangIndex = WangId::TopLeft;
                    break;
                default:
                    break;
            }
            break;
    }
}

void WangPainter::paint(MapDocument *mapDocument, TileLayer *tileLayer, bool useTileMode) {
    updateStamp(mCurrentPosition, mapDocument, tileLayer, useTileMode);
    for (int j = 0; j < mStamp->height(); ++j) {
        for (int i = 0; i < mStamp->width(); ++i) {
            Cell cell = mStamp->cellAt(i, j);
            if (cell.tileset() == nullptr)
            {
                continue;
            }
            if (cell.tile() == nullptr)
            {
                continue;
            }
            
            tileLayer->setCell(mStamp->x() + i, mStamp->y() + j, cell);
        }
    }
}

void WangPainter::updateStamp(QPoint pos, MapDocument *mapDocument, TileLayer *back, bool useTileMode) {
    mStamp = SharedTileLayer::create(QString(), 0, 0, 0, 0);
    WangFiller wangFiller{*mWangSet, mapDocument->renderer()};
    wangFiller.setCorrectionsEnabled(true);
    WangFiller::FillRegion fill;
    updateStampAt(mapDocument, fill, pos, useTileMode);
    wangFiller.fillRegion(*mStamp, *back, fill.region, fill.grid);

    QRegion brushRegion = mStamp->region([](const Cell &cell)
                                            { return cell.checked(); });
    brushRegion.translate(back->position());
    QRect brushRect = brushRegion.boundingRect();
    mStamp->setPosition(brushRect.topLeft());
    mStamp->resize(brushRect.size(), -brushRect.topLeft());
}

// NOTE: This is currently duplicated from WangBrush::updateBrushAt.
void WangPainter::updateStampAt(MapDocument *mapDocument, WangFiller::FillRegion &fill, QPoint pos, bool useTileMode) {
    auto hexgonalRenderer = dynamic_cast<HexagonalRenderer *>(mapDocument->renderer());
    Grid<WangFiller::CellInfo> &grid = fill.grid;
    QRegion &region = fill.region;

    if (useTileMode)
    {
        // array of adjacent positions which is assigned based on map orientation.
        QPoint adjacentPositions[WangId::NumIndexes];
        if (hexgonalRenderer)
        {
            adjacentPositions[0] = hexgonalRenderer->topRight(pos.x(), pos.y());
            adjacentPositions[2] = hexgonalRenderer->bottomRight(pos.x(), pos.y());
            adjacentPositions[4] = hexgonalRenderer->bottomLeft(pos.x(), pos.y());
            adjacentPositions[6] = hexgonalRenderer->topLeft(pos.x(), pos.y());

            if (mapDocument->map()->staggerAxis() == Map::StaggerX)
            {
                adjacentPositions[1] = pos + QPoint(2, 0);
                adjacentPositions[3] = pos + QPoint(0, 1);
                adjacentPositions[5] = pos + QPoint(-2, 0);
                adjacentPositions[7] = pos + QPoint(0, -1);
            }
            else
            {
                adjacentPositions[1] = pos + QPoint(1, 0);
                adjacentPositions[3] = pos + QPoint(0, 2);
                adjacentPositions[5] = pos + QPoint(-1, 0);
                adjacentPositions[7] = pos + QPoint(0, -2);
            }
        }
        else
        {
            for (int i = 0; i < WangId::NumIndexes; ++i)
                adjacentPositions[i] = pos + aroundTilePoints[i];
        }

        WangFiller::CellInfo center = grid.get(pos);

        switch (mBrushMode)
        {
        case WangBrush::BrushMode::PaintCorner:
            for (int i = 0; i < WangId::NumCorners; ++i)
            {
                center.desired.setCornerColor(i, mCurrentColor);
                center.mask.setCornerColor(i, WangId::INDEX_MASK);
            }
            break;
        case WangBrush::BrushMode::PaintEdge:
            for (int i = 0; i < WangId::NumEdges; ++i)
            {
                center.desired.setEdgeColor(i, mCurrentColor);
                center.mask.setEdgeColor(i, WangId::INDEX_MASK);
            }
            break;
        case WangBrush::BrushMode::PaintEdgeAndCorner:
            for (int i = 0; i < WangId::NumIndexes; ++i)
            {
                center.desired.setIndexColor(i, mCurrentColor);
                center.mask.setIndexColor(i, WangId::INDEX_MASK);
            }
            break;
        case WangBrush::BrushMode::Idle:
            break;
        }

        region += QRect(pos, QSize(1, 1));
        grid.set(pos, center);

        for (int i = 0; i < WangId::NumIndexes; ++i)
        {
            const bool isCorner = WangId::isCorner(i);
            if (mBrushMode == WangBrush::BrushMode::PaintEdge && isCorner)
                continue;

            QPoint p = adjacentPositions[i];
            WangFiller::CellInfo adjacent = grid.get(p);

            // Mark the opposite side or corner of the adjacent tile
            if (isCorner || (mBrushMode == WangBrush::BrushMode::PaintEdge || mBrushMode ==WangBrush::BrushMode::PaintEdgeAndCorner))
            {
                adjacent.desired.setIndexColor(WangId::oppositeIndex(i), mCurrentColor);
                adjacent.mask.setIndexColor(WangId::oppositeIndex(i), WangId::INDEX_MASK);
            }

            // Mark the touching corners of the adjacent tile
            if (!isCorner && (mBrushMode == WangBrush::BrushMode::PaintCorner || mBrushMode == WangBrush::BrushMode::PaintEdgeAndCorner))
            {
                adjacent.desired.setIndexColor((i + 3) % WangId::NumIndexes, mCurrentColor);
                adjacent.desired.setIndexColor((i + 5) % WangId::NumIndexes, mCurrentColor);
                adjacent.mask.setIndexColor((i + 3) % WangId::NumIndexes, WangId::INDEX_MASK);
                adjacent.mask.setIndexColor((i + 5) % WangId::NumIndexes, WangId::INDEX_MASK);
            }

            region += QRect(p, QSize(1, 1));
            grid.set(p, adjacent);
        }
    }
    else
    {
        if (mWangIndex == WangId::NumIndexes)
            return;

        auto brushMode = mBrushMode;

        if (brushMode == WangBrush::BrushMode::PaintEdgeAndCorner)
            brushMode = WangId::isCorner(mWangIndex) ? WangBrush::BrushMode::PaintCorner : WangBrush::BrushMode::PaintEdge;

        switch (brushMode)
        {
            case WangBrush::BrushMode::PaintCorner:
            {
                QPoint adjacentPoints[WangId::NumCorners];

                if (hexgonalRenderer)
                {
                    adjacentPoints[0] = hexgonalRenderer->topRight(pos.x(), pos.y());
                    adjacentPoints[1] = pos;
                    adjacentPoints[2] = hexgonalRenderer->topLeft(pos.x(), pos.y());
                    adjacentPoints[3] = hexgonalRenderer->topRight(adjacentPoints[2].x(), adjacentPoints[2].y());
                }
                else
                {
                    for (int i = 0; i < WangId::NumCorners; ++i)
                        adjacentPoints[i] = pos + aroundVertexPoints[i];
                }

                for (int i = 0; i < WangId::NumCorners; ++i)
                {
                    const QPoint p = adjacentPoints[i];

                    region += QRect(p, QSize(1, 1));

                    WangFiller::CellInfo adjacent = grid.get(p);
                    adjacent.desired.setCornerColor((i + 2) % 4, mCurrentColor);
                    adjacent.mask.setCornerColor((i + 2) % 4, WangId::INDEX_MASK);

                    grid.set(p, adjacent);
                }

                break;
            }
            case WangBrush::BrushMode::PaintEdge:
            {
                QPoint dirPoint;
                if (hexgonalRenderer)
                {
                    switch (mWangIndex)
                    {
                    case WangId::Top:
                        dirPoint = hexgonalRenderer->topRight(pos.x(), pos.y());
                        break;
                    case WangId::Right:
                        dirPoint = hexgonalRenderer->bottomRight(pos.x(), pos.y());
                        break;
                    case WangId::Bottom:
                        dirPoint = hexgonalRenderer->bottomLeft(pos.x(), pos.y());
                        break;
                    case WangId::Left:
                        dirPoint = hexgonalRenderer->topLeft(pos.x(), pos.y());
                        break;
                    default: // Other color indexes not handled when painting edges
                        break;
                    }
                }
                else
                {
                    dirPoint = pos + aroundTilePoints[mWangIndex];
                }

                region += QRect(pos, QSize(1, 1));
                region += QRect(dirPoint, QSize(1, 1));

                {
                    WangFiller::CellInfo info = grid.get(pos);
                    info.desired.setIndexColor(mWangIndex, mCurrentColor);
                    info.mask.setIndexColor(mWangIndex, WangId::INDEX_MASK);
                    grid.set(pos, info);
                }
                {
                    WangFiller::CellInfo info = grid.get(dirPoint);
                    info.desired.setIndexColor(WangId::oppositeIndex(mWangIndex), mCurrentColor);
                    info.mask.setIndexColor(WangId::oppositeIndex(mWangIndex), WangId::INDEX_MASK);
                    grid.set(dirPoint, info);
                }

                break;
            }
            case WangBrush::BrushMode::PaintEdgeAndCorner: // Handled before switch
            case WangBrush::BrushMode::Idle:
                break;
        }
    }
}

}
