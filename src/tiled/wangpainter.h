#pragma once

#include "wangset.h"
#include "wangfiller.h"
#include "tilelayer.h"
#include "mapdocument.h"

namespace Tiled {
    class WangPainter {
    public:
        enum BrushMode {
            PaintCorner,
            PaintEdge,
            PaintEdgeAndCorner,
            Idle // no valid color selected
        };

        WangPainter();
        virtual ~WangPainter();

        BrushMode brushMode();

        void setWangSet(const WangSet *wangSet);
        void setTerrain(WangFiller::FillRegion &fill, MapDocument *mapDocument, int color, QPoint pos, WangId::Index directionToGenerate);
        void setTerrain(MapDocument *mapDocument, int color, QPoint pos, WangId::Index directionToGenerate);
        void clear();
        void commit(MapDocument *mapDocument, TileLayer *tileLayer);

    private:
        void setColor(int color);
        WangId::Index getDesiredDirection(WangId::Index initialDirection);
        void generateTerrainAt(MapDocument *mapDocument, WangFiller::FillRegion &fill, int color, QPoint pos, WangId::Index direction, bool useTileMode = false);

        const WangSet *mWangSet;

        int mCurrentColor = 0;
        WangFiller::FillRegion mCurrentFill;
        BrushMode mBrushMode = BrushMode::Idle;
    };
}
