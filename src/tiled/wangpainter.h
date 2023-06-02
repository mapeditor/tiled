#pragma once

#include "wangset.h"
#include "wangbrush.h"
#include "tilelayer.h"
#include "mapdocument.h"

namespace Tiled {
    class WangPainter {
    public:
        WangPainter();
        virtual ~WangPainter();

        void setWangSet(const WangSet *wangSet);
        void setTerrain(MapDocument *mapDocument, int color, QPoint pos, WangId::Index directionToGenerate);
        void commit(MapDocument *mapDocument, TileLayer *tileLayer);

    private:
        void setColor(int color);
        WangId::Index getDesiredDirection(WangId::Index initialDirection);
        void generateTerrainAt(MapDocument *mapDocument, WangFiller::FillRegion &fill, int color, QPoint pos, WangId::Index direction, bool useTileMode = false);

        const WangSet *mWangSet;

        int mCurrentColor = 0;
        WangFiller::FillRegion mCurrentFill;
        WangBrush::BrushMode mBrushMode = WangBrush::BrushMode::Idle;
    };
}
