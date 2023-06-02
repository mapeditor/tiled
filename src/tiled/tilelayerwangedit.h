#pragma once

#include "editablewangset.h"
#include "tilelayer.h"
#include "wangpainter.h"

#include <QObject>

namespace Tiled {

class EditableTileLayer;

class TileLayerWangEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Tiled::EditableTileLayer *target READ target)
    Q_PROPERTY(bool mergeable READ isMergeable WRITE setMergeable)

public:
    explicit TileLayerWangEdit(EditableTileLayer *tileLayer,
                           EditableWangSet *wangSet,
                           QObject *parent = nullptr);
    ~TileLayerWangEdit() override;

    /**
     * Sets whether this edit can be merged with a previous edit.
     *
     * Calling apply() automatically set this edit to be mergeable, so that
     * edits are merged when this object is reused.
     */
    void setMergeable(bool mergeable);
    bool isMergeable() const;

    EditableTileLayer *target() const;

public slots:
    void setTerrain(int x, int y, int color, WangId::Index direction = WangId::Left);
    void apply();

private:
    EditableTileLayer *mTargetLayer;
    TileLayer mChanges;
    bool mMergeable = false;
    WangPainter *mWangPainter = nullptr;
};


inline void TileLayerWangEdit::setMergeable(bool mergeable)
{
    mMergeable = mergeable;
}

inline bool TileLayerWangEdit::isMergeable() const
{
    return mMergeable;
}

inline EditableTileLayer *TileLayerWangEdit::target() const
{
    return mTargetLayer;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::TileLayerWangEdit*)
