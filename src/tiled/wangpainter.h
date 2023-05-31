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
        void setColor(int color);
        void setPosition(QPoint pos);
        void setDirection(WangId::Index directionToGenerate);
        void paint(MapDocument *mapDocument, TileLayer *tileLayer, bool useTileMode = false);

    private:
        void updateStampAt(MapDocument *mapDocument, FillRegion &fill, QPoint pos, bool useTileMode = false);
        void updateStamp(QPoint pos, MapDocument *mapDocument, TileLayer *back, bool useTileMode = false);

        const WangSet *mWangSet;
        SharedTileLayer mStamp;
        QRegion mStampRegion;
        QPoint mCurrentPosition;
        
        int mCurrentColor = 0;
        WangId::Index mWangIndex = WangId::Top;

        WangBrush::BrushMode mBrushMode = WangBrush::BrushMode::Idle;
    };
}
